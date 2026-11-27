#!/usr/bin/env python3
"""Mixed link: the firmware with the C functions (nonmatching/) in place of their assembly.

  mixlink.py [--only A,B,..] [--skip A,B,..] [--max N]     (after `make check nonmatching`)
      -> build/mix/app.bin, build/mix/app.map, build/mix/reloc.json, build/mix/report.txt

Each function that has a C file is linked from its GCC object (build/nonmatching/func_X.o,
the object `make emutest` checked) instead of from asm/func_X.s. The rest of the image is the
relocatable assembly of tools/split.py, so the linker places everything and the image is no
longer byte-identical to the original (it is NOT the matching build: `make check` still is).

An assembly unit can only leave when nothing that stays depends on where it is:
  - a PC-relative operand of a unit that stays reads inside it (shared literal pool), or
    a branch that stays targets one of its interior labels;
  - a raw (.inst) branch or load of a unit that stays crosses it or reaches it (fixed offset):
    then every unit between the two stays as well;
  - a unit that stays runs on into it (no return or branch at its end).
Those units keep their assembly (the report lists why). Data that lives inside a function
that leaves (inline strings, tables in its holes) and that something else still names
(d_XXXXXXXX) is kept as a small data section of its own.

Names the C uses that nothing defines resolve like tools/nmbuild.py does: RAM addresses
(past the ZI base 0x400954A4) as absolute symbols, image addresses as aliases of the symbol
now at that place. reloc.json maps each stock function entry to its new address, for the
emulator hooks (`node tools/sim/tests/boot.mjs --reloc build/mix/reloc.json`).
"""
import argparse
import bisect
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from tools import split, srcindex, nmbuild    # noqa: E402
from tools.split import BASE, RELOC_END, disasm_one, BR_IDS, A, data_lines  # noqa: E402

OUT = ROOT / "build/mix"
CROSS = "arm-none-eabi-"
NM_DIR = ROOT / "build/nonmatching"


def parse_addrs(s):
    return {int(x, 16) for x in s.split(",") if x} if s else set()


class Analysis:
    def __init__(self):
        im = split.Image()
        units = split.build_units(im)
        fp = ROOT / "build/split/forced.json"
        forced = set(json.loads(fp.read_text())) if fp.exists() else set()
        em = split.Emitter(im, units, forced)
        self.im, self.units, self.em = im, units, em
        self.starts = [u[0] for u in units]
        self.cluster_of = {}
        for k, (i, j) in enumerate(em.clusters):
            for x in range(i, j + 1):
                self.cluster_of[x] = k
        self._edges()

    def idx(self, a):
        return bisect.bisect_right(self.starts, a) - 1

    def _edges(self):
        """sym[i] = units i needs to exist; fixed[i] = (lo, hi) unit ranges that must stay
        exactly as they are if i stays; pool[i] = literals and inline data of other units that i
        reads PC-relative (kept as a stub where their unit was, see pool_stub);
        data_users[v] = who names the d_ label at v"""
        im, em, units = self.im, self.em, self.units
        self.sym = {i: set() for i in range(len(units))}
        self.fixed = {i: [] for i in range(len(units))}
        self.pool = {i: set() for i in range(len(units))}
        self.why = {}
        for i, u in enumerate(units):
            if u[2] != "func":
                continue
            th = u[3]["mode"] == "thumb"
            lin = [(a, s) for a, s in em._linear(u[3]) if s > 0]
            for a, sz in em._linear(u[3]):
                if sz < 0:
                    continue
                ins = disasm_one(im, a, th)
                t, raw = None, False
                if a in em.pcrel:
                    t, raw = em.pcrel[a], a in em.forced
                elif ins.id in BR_IDS and ins.operands and ins.operands[-1].type == A.ARM_OP_IMM:
                    t = ins.operands[-1].imm
                    if not (BASE <= t < BASE + len(im.img)):
                        continue
                    raw = em.ins_text(a, ins, th, u[0]).startswith(".inst")
                    if not raw and t in im.funcs:
                        continue          # call or tail call by name: the C defines it
                if t is None:
                    continue
                j = self.idx(t)
                if j == i:
                    continue
                if raw:
                    self.fixed[i].append((min(i, j), max(i, j)))
                elif a in em.pcrel and t not in em.func_of:
                    self.pool[i].add(t)       # a literal or inline data: can stay without its code
                    continue
                else:
                    self.sym[i].add(j)
                self.why.setdefault((i, j), f"0x{a:08x} -> 0x{t:08x}{' (raw)' if raw else ''}")
            # runs on into the next unit
            if lin and lin[-1][0] + lin[-1][1] == u[1] and i + 1 < len(units) and \
                    not split.is_terminal(disasm_one(im, lin[-1][0], th)):
                self.fixed[i].append((i, i + 1))
                self.why.setdefault((i, i + 1), "falls through")
        # who names each data label: literal words and movw/movt of a unit, the relative word, the C
        self.data_users = {}
        for la, sz in im.lit.items():
            if sz == 8:
                continue
            for k in range(0, sz, 4):
                v = im.u32(la + k)
                if v in em.data_refs:
                    self.data_users.setdefault(v, set()).add(self.idx(la))
        for a, (_, v, _) in em.movsym.items():
            if v in em.data_refs:
                self.data_users.setdefault(v, set()).add(self.idx(a))
        for v in em.c_refs:
            if v in em.data_refs:
                self.data_users.setdefault(v, set()).add("C")


def choose(an, cands):
    """largest set of candidate units that can leave (see module doc) and, for the others,
    {unit: (unit that needs it, why)}"""
    out, reasons = set(cands), {}
    while True:
        drop = {}
        for i in range(len(an.units)):
            if i in out:
                continue
            for j in an.sym[i]:
                if j in out:
                    drop.setdefault(j, (i, an.why[(i, j)]))
            for lo, hi in an.fixed[i]:
                for j in range(lo, hi + 1):
                    if j in out:
                        drop.setdefault(j, (i, an.why.get((i, j)) or an.why.get((i, lo)) or
                                            an.why.get((i, hi)) or "fixed range"))
        if not drop:
            return out, reasons
        out -= set(drop)
        reasons.update(drop)


def runs_of(an, u):
    """non-instruction byte runs [(lo, hi)] of a function unit"""
    f = u[3]
    pos, out = u[0], []
    for a, sz in an.em._linear(f):
        if a > pos:
            out.append((pos, a))
        pos = a + abs(sz)
    if pos < u[1]:
        out.append((pos, u[1]))
    return out


D_RE = re.compile(r"\bd_([0-9a-f]{8})\b")


def item_range(an, u, t):
    """bytes to keep for a PC-relative target t inside function unit u: the literal itself, or
    the whole data run around it (a string or table reached with adr)"""
    if t in an.im.lit:
        return t, t + an.im.lit[t]
    return next((lo, hi) for lo, hi in runs_of(an, u) if lo <= t < hi)


def emit_data(an, lo, hi, local=frozenset()):
    """data lines for [lo, hi) with .Lp_ labels at 'local' and d_ labels at the data symbols"""
    em, im = an.em, an.im
    labels = {x for x in em.data_refs if lo <= x < hi}
    cuts = frozenset(x for x in labels | set(local) if x > lo)
    lines = ["    .balign 4"] + ([f"    .space {lo & 3}"] if lo & 3 else [])
    for a, t, _ in data_lines(im, lo, hi, em.ptrname, cuts):
        if a in labels:
            lines += [f"    .global d_{a:08x}", f"d_{a:08x}:"]
        if a in local:
            lines.append(f".Lp_{a:08x}:")
        lines.append(f"    {t}")
    return lines, labels


def pool_stub(an, j, targets):
    """what stays of unit j (linked from C) for the units that read its literal pool"""
    u = an.units[j]
    spans = sorted({item_range(an, u, t) for t in targets})
    merged = []
    for lo, hi in spans:
        if merged and lo <= merged[-1][1]:
            merged[-1] = (merged[-1][0], max(hi, merged[-1][1]))
        else:
            merged.append((lo, hi))
    lines, labels = [f"/* literals of {an.im.fname[u[0]]} (linked from C) read by the assembly around it */"], set()
    for lo, hi in merged:
        ls, lb = emit_data(an, lo, hi, {t for t in targets if lo <= t < hi})
        lines += ls
        labels |= lb
    return lines, labels


def remnants(an, gone, named, done):
    """data sections for the d_ labels inside units that leave and are still named: by a unit
    that stays, by the C, or by data kept already ('named' = d_ names used by stubs); 'done' =
    labels the stubs define already"""
    em, im = an.em, an.im
    need = set(named)
    for v, users in an.data_users.items():
        if any(x == "C" or x not in gone for x in users):
            need.add(v)
    secs, files, have = {}, [], set(done)
    while True:
        new = {v for v in need - have if an.idx(v) in gone and v not in have}
        if not new:
            break
        for v in sorted(new):
            u = an.units[an.idx(v)]
            lo, hi = next((lo, hi) for lo, hi in runs_of(an, u) if lo <= v < hi)
            if (lo, hi) in secs:
                continue
            ls, labels = emit_data(an, lo, hi)
            secs[(lo, hi)] = (u, ls, labels)
            have |= labels
            need |= {int(m, 16) for l in ls for m in D_RE.findall(l)}
        have |= new
    for (lo, hi), (u, ls, labels) in sorted(secs.items()):
        lines = [f"/* 0x{lo:08x}-0x{hi:08x}: data inside {im.fname[u[0]]}, which is linked from C */",
                 f'    .section .rodata.d_{lo:08x}, "a", %progbits'] + ls
        files.append((f"rem_{lo:08x}.s", lines, len(labels)))
    return files


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", help="comma-separated stock addresses: replace only these")
    ap.add_argument("--skip", help="comma-separated stock addresses to keep in assembly")
    ap.add_argument("--max", type=int, help="replace at most the first N (address order)")
    a = ap.parse_args()

    srcs = srcindex.sources()
    infos = {}
    for orig in srcs:
        js = NM_DIR / f"func_{orig:08x}.json"
        if not js.exists():
            sys.exit(f"{js} missing: run `make nonmatching` first")
        infos[orig] = json.loads(js.read_text())
    an = Analysis()
    im, em, units = an.im, an.em, an.units
    uidx = {u[0]: i for i, u in enumerate(units) if u[2] == "func"}
    want = sorted(o for o in srcs if o in uidx)
    only, skip = parse_addrs(a.only), parse_addrs(a.skip)
    if only:
        want = [o for o in want if o in only]
    want = [o for o in want if o not in skip]
    if a.max is not None:
        want = want[:a.max]
    gone, reasons = choose(an, {uidx[o] for o in want})
    gone_addr = {units[i][0] for i in gone}

    OUT.mkdir(parents=True, exist_ok=True)
    sec_dir = OUT / "sections"
    sec_dir.mkdir(exist_ok=True)
    for old in sec_dir.glob("*"):
        old.unlink()
    # assembly: every cluster with the units that stay, plus the remnant data
    objs, ld_lines = [], []
    stubs = {}                           # gone unit -> literals that units staying read from it
    for i in range(len(units)):
        if i not in gone:
            for t in an.pool[i]:
                if an.idx(t) in gone:
                    stubs.setdefault(an.idx(t), set()).add(t)
    stub_labels, stub_names = set(), set()
    for k, (i, j) in enumerate(em.clusters):
        keep = [x for x in range(i, j + 1) if x not in gone]
        c0 = units[i][0]
        if c0 == RELOC_END:
            continue                     # bytes past the ZI base (packer residue): left out
        if keep:
            body = [f'    .section .text.{c0:08x}, "ax", %progbits']
            for x in range(i, j + 1):
                u = units[x]
                if x in gone:
                    if stubs.get(x):
                        ls, lb = pool_stub(an, x, stubs[x])
                        body += ls
                        stub_labels |= lb
                        stub_names |= {int(m, 16) for l in ls for m in D_RE.findall(l)}
                    continue
                if x != keep[0] and not (u[2] == "func" and u[3]["mode"] == "thumb"):
                    body.append("    .balign 4")
                name = "vectors" if u[2] == "vectors" else (f"data_{u[0]:08x}" if u[2] == "blob" else f"func_{u[0]:08x}")
                body.append(f'    .include "asm/{name}.s"')
            (sec_dir / f"sec_{c0:08x}.s").write_text("\n".join(body) + "\n")
            objs.append(sec_dir / f"sec_{c0:08x}.o")
            ld_lines.append(f"        KEEP(*(.text.{c0:08x}))")
        for x in range(i, j + 1):
            if x in gone:
                o = NM_DIR / f"func_{units[x][0]:08x}.o"
                objs.append(o)
                rel = o.relative_to(ROOT).as_posix()
                ld_lines.append(f"        KEEP({rel}(.text .text.* .rodata .rodata.* .data .data.* .bss .bss.* COMMON))")
    rem = remnants(an, gone, stub_names, stub_labels)
    for fn, lines, _ in rem:
        (sec_dir / fn).write_text("\n".join(lines) + "\n")
        objs.append(sec_dir / fn.replace(".s", ".o"))
        ld_lines.append(f"        KEEP(*(.rodata.d_{fn[4:12]}))")
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor() as ex:
        list(ex.map(lambda s: subprocess.run([CROSS + "as", *split.ASFLAGS, "-I.", "-o", str(s.with_suffix(".o")),
                                              str(s)], check=True, cwd=ROOT, capture_output=True),
                    sorted(sec_dir.glob("*.s"))))

    # names: what every object defines and needs
    def nm(o):
        r = subprocess.run([CROSS + "nm", str(o)], capture_output=True, text=True, check=True).stdout
        d, u = set(), set()
        for l in r.splitlines():
            f = l.split()
            if len(f) == 2 and f[0] == "U":
                u.add(f[1])
            elif len(f) == 3 and f[1] in "TtDdBbRrA" and f[1].isupper():
                d.add(f[2])
        return d, u
    defined, undef, dup = {}, set(), []
    for o in objs:
        d, u = nm(o)
        for n in d:
            if n in defined:
                dup.append((n, defined[n], o))
            defined[n] = o
        undef |= u
    syms = nmbuild.original_symbols()
    aliases = []
    # an asm caller may name the stock function differently from the C file
    for x in sorted(gone):
        an_name, c_name = im.fname[units[x][0]], infos[units[x][0]]["name"]
        if an_name != c_name and an_name not in defined:
            aliases.append(f"{an_name} = {c_name};")
            defined[an_name] = "alias"
    missing = []
    for n in sorted(undef - set(defined)):
        v = syms.get(n)
        if v is None:
            missing.append(n)
        elif (v & ~1) >= RELOC_END or (v & ~1) < BASE:
            aliases.append(f"{n} = 0x{v:08X};")
        else:
            t = v & ~1
            s = infos[t]["name"] if t in gone_addr else em.ptrname(v)
            aliases.append(f"{n} = {s};")
    if missing:
        sys.exit("undefined and not in the symbol tables: " + ", ".join(missing))
    if dup:
        sys.exit("defined twice: " + ", ".join(f"{n} ({p.name}, {q.name})" for n, p, q in dup[:20]))

    lds = OUT / "mix.ld"
    lds.write_text("""OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)
SECTIONS
{
    .text 0x40000000 : {
%s
    }
    __image_end = .;
    ASSERT(__image_end <= 0x%08X, "mixed image runs into the ZI region")
    /DISCARD/ : { *(.text.400954a4) *(.ARM.attributes) *(.comment) *(.note*) *(.ARM.exidx*) }
}
%s
""" % ("\n".join(ld_lines), RELOC_END, "\n".join(aliases)))
    (OUT / "objs.rsp").write_text("\n".join(o.relative_to(ROOT).as_posix() for o in objs))
    subprocess.run([CROSS + "ld", "-T", str(lds), "-Map=" + str(OUT / "app.map"), "-o", str(OUT / "app.elf"),
                    "@" + str(OUT / "objs.rsp")], check=True, cwd=ROOT)
    subprocess.run([CROSS + "objcopy", "-O", "binary", str(OUT / "app.elf"), str(OUT / "app.bin")], check=True)

    # where every stock function went
    st = subprocess.run([CROSS + "nm", str(OUT / "app.elf")], capture_output=True, text=True, check=True).stdout
    val = {l.split()[2]: int(l.split()[0], 16) for l in st.splitlines() if len(l.split()) == 3}
    spans = []
    for x, u in enumerate(units):
        if u[2] == "func":
            n = infos[u[0]]["name"] if x in gone else im.fname[u[0]]
            if n in val:
                nv = val[n] & ~1
                spans.append([u[0], u[0] + 1 if x in gone else u[1], nv - u[0]])
        elif u[2] == "vectors":
            spans.append([u[0], u[1], val["vectors"] - u[0]])
    (OUT / "reloc.json").write_text(json.dumps(spans))

    size = (OUT / "app.bin").stat().st_size
    rep = [f"C functions available: {len(srcs)}; asked for: {len(want)}; linked from C: {len(gone)}",
           f"kept in assembly although they have C: {len(want) - len(gone)}",
           f"literal stubs (pools read by assembly that stays): {len(stubs)} units, "
           f"{sum(len(v) for v in stubs.values())} literals",
           f"remnant data sections: {len(rem)} ({sum(n for _, _, n in rem)} labels)",
           f"names resolved by alias or address: {len(aliases)}",
           f"image: {size} bytes (0x{size:X}), ends at 0x{BASE + size:08X}; ZI base 0x{RELOC_END:08X}, "
           f"free 0x{RELOC_END - BASE - size:X}", ""]
    for j, (i, w) in sorted(reasons.items()):
        if j in gone:
            continue
        rep.append(f"  asm kept 0x{units[j][0]:08x} {im.fname[units[j][0]]}: needed by "
                   f"0x{units[i][0]:08x} {im.fname.get(units[i][0], units[i][2])} ({w})")
    (OUT / "report.txt").write_text("\n".join(rep) + "\n")
    print("\n".join(rep[:6]))


if __name__ == "__main__":
    main()
