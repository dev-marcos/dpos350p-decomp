#!/usr/bin/env python3
"""Split the application image into per-function GNU-as files + data blobs.

  split.py gen      build/analysis/map.json + build/extract/app.bin
                    -> asm/*.s, data/blobs.csv, ld/sections.ld   (committed outputs)
  split.py extract  build/extract/app.bin + data/blobs.csv -> data/*.bin (build time;
                    FNIRSI bytes are never committed)

Units, in address order:
  * one file per function, asm/func_XXXXXXXX.s (or asm/<name>.s for named ones), holding
    the function plus everything up to the next unit (literal pools, inline strings, padding)
  * one file per large data area, asm/data_XXXXXXXX.s, which .incbin's data/<name>.bin

Instruction text comes from Capstone. Branch/call targets become labels. Each instruction
is verified against the original bytes with an `as` listing of the whole image; anything
that does not reassemble identically is emitted as `.inst` (ARM) or `.inst.n/.inst.w` (Thumb).
"""
import csv
import hashlib
import json
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BASE = 0x40000000

# Large data areas become binary blobs (see docs/MEMORY_MAP.md).
BLOB_AREAS = [
    (0x40000040, 0x40019AF4, "fonts"),
    (0x4006DDEC, 0x4008CB28, "graphics"),
    (0x4008CFB8, 0x400954A4, "rodata"),
    (0x400954A4, 0x400957E0, "packer_garbage"),
]

AS = "arm-none-eabi-as"
ASFLAGS = ["-mcpu=cortex-a7", "-mfpu=neon-vfpv4"]


def ident(name):
    n = re.sub(r"[^A-Za-z0-9_$]", "_", name)
    if not re.match(r"[A-Za-z_]", n):
        n = "f_" + n
    return n


class Image:
    def __init__(self):
        self.img = (ROOT / "build/extract/app.bin").read_bytes()
        self.map = json.loads((ROOT / "build/analysis/map.json").read_text())
        self.funcs = {f["start"]: f for f in self.map["functions"]}
        self.syms = {}
        for r in csv.DictReader(open(ROOT / "symbols/symbols_2025-06-19.csv")):
            if r["type"].startswith("function"):
                self.syms[int(r["address"], 16)] = r["name"]
        # unique, valid names
        seen = {}
        self.fname = {}
        for a in sorted(self.funcs):
            n = ident(self.syms.get(a, f"func_{a:08x}"))
            if n in seen:
                n = f"{n}_{a:08x}"
            seen[n] = a
            self.fname[a] = n
        self.lit = {}
        for f in self.map["functions"]:
            for la, sz in f["literals"]:
                self.lit[la] = max(self.lit.get(la, 0), sz)

    def u32(self, a):
        return struct.unpack_from("<I", self.img, a - BASE)[0]

    def bytes(self, a, n):
        return self.img[a - BASE:a - BASE + n]


def build_units(im):
    """[(start, end, kind, func_or_blobname)] covering the whole image."""
    units = []
    starts = sorted(im.funcs)
    blob_at = {lo: (hi, n) for lo, hi, n in BLOB_AREAS}
    # cut points
    cuts = sorted(set(starts) | {lo for lo, _, _ in BLOB_AREAS} | {hi for _, hi, _ in BLOB_AREAS}
                  | {BASE, BASE + 0x40, BASE + len(im.img)})
    for i, a in enumerate(cuts[:-1]):
        e = cuts[i + 1]
        if a in blob_at:
            units.append((a, blob_at[a][0], "blob", blob_at[a][1]))
        elif any(lo < a < hi for lo, hi, _ in BLOB_AREAS):
            continue
        elif a == BASE:
            units.append((a, e, "vectors", None))
        elif a in im.funcs:
            units.append((a, e, "func", im.funcs[a]))
        else:
            units.append((a, e, "gap", None))
    # merge 'gap' units into the preceding unit (code-area leftovers before first func)
    out = []
    for u in units:
        if u[2] == "gap" and out and out[-1][2] in ("func", "vectors"):
            p = out[-1]
            out[-1] = (p[0], u[1], p[2], p[3])
        else:
            out.append(u)
    # sanity
    pos = BASE
    for u in out:
        assert u[0] == pos, (hex(u[0]), hex(pos))
        pos = u[1]
    assert pos == BASE + len(im.img)
    return out


# ------------------------------------------------------------------ disassembly
from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB  # noqa: E402
from capstone import arm as A  # noqa: E402

MD = {False: Cs(CS_ARCH_ARM, CS_MODE_ARM), True: Cs(CS_ARCH_ARM, CS_MODE_THUMB)}
for _m in MD.values():
    _m.detail = True

BR_IDS = {A.ARM_INS_B, A.ARM_INS_BL, A.ARM_INS_BLX, A.ARM_INS_CBZ, A.ARM_INS_CBNZ}


def disasm_one(im, a, thumb):
    return next(MD[thumb].disasm(im.bytes(a, 4), a, 1), None)


def inst_directive(ins_bytes, thumb):
    if not thumb:
        return f".inst 0x{struct.unpack('<I', ins_bytes)[0]:08x}"
    if len(ins_bytes) == 2:
        return f".inst.n 0x{struct.unpack('<H', ins_bytes)[0]:04x}"
    hi, lo = struct.unpack("<HH", ins_bytes)
    return f".inst.w 0x{hi:04x}{lo:04x}"


def data_lines(im, a, e, ptrname):
    """Bytes [a,e) that are not instructions: pool words (maybe symbolised), zeros, bytes."""
    out = []
    x = a
    while x < e:
        if x in im.lit and im.lit[x] >= 4 and x + 4 <= e and (x & 3) == 0:
            for k in range(0, im.lit[x] if x + im.lit[x] <= e else 4, 4):
                v = im.u32(x + k)
                out.append((x + k, f".word {ptrname(v)}", 4))
            x += max(4, min(im.lit[x], e - x)) // 4 * 4
            continue
        z = x
        while z < e and im.img[z - BASE] == 0 and z not in im.lit:
            z += 1
        if z - x >= 4:
            out.append((x, f".space {z - x}", z - x))
            x = z
            continue
        y = x + 1
        while y < e and (y - x) < 16 and y not in im.lit and not (im.img[y - BASE] == 0 and
                                                                     im.img[y - BASE:y - BASE + 4] == b"\0\0\0\0"):
            y += 1
        out.append((x, ".byte " + ", ".join(f"0x{b:02x}" for b in im.bytes(x, y - x)), y - x))
        x = y
    return out


class Emitter:
    def __init__(self, im, units, forced):
        self.im, self.units, self.forced = im, units, forced
        # global label targets: every direct branch target that is not a function start
        self.func_of = {}
        self.labels = {}           # addr -> label name (global, since branches may cross files)
        self.thumb_at = {}
        for u in units:
            if u[2] == "func":
                f = u[3]
                for a, s in [(a, s) for a, s in self._insns(f)]:
                    self.func_of[a] = f["start"]
        self.mode_conflicts = set()  # (insn, target) where target's mode disagrees
        self.global_labels = set()   # interior targets referenced from another unit
        for u in units:
            if u[2] != "func":
                continue
            f = u[3]
            th = f["mode"] == "thumb"
            for a, _ in self._insns(f):
                ins = disasm_one(im, a, th)
                if ins and ins.id in BR_IDS and ins.operands and ins.operands[-1].type == A.ARM_OP_IMM:
                    t = ins.operands[-1].imm
                    if t in im.funcs:
                        continue
                    if t in self.func_of:
                        self.labels[t] = f"loc_{t:08x}"
                        if self.func_of[t] != f["start"]:
                            self.global_labels.add(t)

    def _insns(self, f):
        return self._linear(f)

    def _linear(self, f):
        if not hasattr(self, "_lin"):
            self._lin = {}
        if f["start"] in self._lin:
            return self._lin[f["start"]]
        th = f["mode"] == "thumb"
        holes = [(a, b) for a, b in f["holes"]]
        res = []
        a = f["start"]
        while a < f["end"]:
            h = next(((x, y) for x, y in holes if x <= a < y), None)
            if h:
                a = h[1]
                continue
            ins = disasm_one(self.im, a, th)
            if ins is None:
                res.append((a, -(2 if th else 4)))   # undecodable: raw
                a += 2 if th else 4
                continue
            res.append((a, ins.size))
            a += ins.size
        self._lin[f["start"]] = res
        return res

    def sym_for(self, t, here=None):
        """Symbol for branch target t, seen from the function starting at 'here'."""
        if t == here:
            return f".Lstart_{t:08x}"            # self-reference: local alias, no relocation
        if t in self.im.funcs:
            return self.im.fname[t]
        if t in self.labels:
            if t in self.global_labels and self.func_of.get(t) != here:
                return self.labels[t]
            return f".L{t:08x}"                  # same unit: local alias (no relocation)
        return None

    def cross_unit(self, t, here):
        if t == here:
            return False
        owner = t if t in self.im.funcs else self.func_of.get(t)
        return owner != here

    def ptrname(self, v):
        im = self.im
        if v in im.funcs and im.funcs[v]["mode"] == "arm":
            return im.fname[v]
        if v & 1 and (v - 1) in im.funcs and im.funcs[v - 1]["mode"] == "thumb":
            return im.fname[v - 1]
        return f"0x{v:08x}"

    def ins_text(self, a, ins, th, here=None):
        if a in self.forced:
            return inst_directive(ins.bytes, th)
        text = f"{ins.mnemonic} {ins.op_str}".strip()
        if ins.id in BR_IDS and ins.operands and ins.operands[-1].type == A.ARM_OP_IMM:
            t = ins.operands[-1].imm
            s = self.sym_for(t, here)
            # Thumb: only 32-bit B/BL/BLX/B<c>.W relocate across objects; keep the rest raw
            if s is None or (th and self.cross_unit(t, here) and
                             (ins.size == 2 or ins.id in (A.ARM_INS_CBZ, A.ARM_INS_CBNZ))):
                return inst_directive(ins.bytes, th)
            # target mode must be what the instruction implies, or ld inserts a veneer
            if t in self.im.funcs and t != here:
                target_thumb = self.im.funcs[t]["mode"] == "thumb"
                implied = (not th) if ins.id == A.ARM_INS_BLX else th
                if target_thumb != implied:
                    self.mode_conflicts.add((a, t))
                    return inst_directive(ins.bytes, th)
            text = re.sub(r"#0x[0-9a-f]+$|#-?\d+$", s, text)
        elif "pc" in ins.op_str and ins.id in (A.ARM_INS_ADR,):
            return inst_directive(ins.bytes, th)
        return text

    def unit_lines(self, u):
        """[(addr, text, size_or_None, is_label)] for a unit."""
        im = self.im
        a0, e, kind, obj = u
        L = []
        if kind == "blob":
            L.append((a0, f'.incbin "data/{obj}_{a0:08x}.bin"', e - a0))
            return L
        if kind == "vectors":
            for k in range(8):
                ins = disasm_one(im, a0 + 4 * k, False)
                L.append((a0 + 4 * k, self.ins_text(a0 + 4 * k, ins, False), 4))
            for k in range(8):
                L.append((a0 + 0x20 + 4 * k, f".word {self.ptrname(im.u32(a0 + 0x20 + 4 * k))}", 4))
            if e > a0 + 0x40:
                L += [(x, t, s) for x, t, s in data_lines(im, a0 + 0x40, e, self.ptrname)]
            return L
        f = obj
        th = f["mode"] == "thumb"
        lin = self._linear(f)
        pos = a0
        for a, sz in lin:
            if a > pos:
                L += data_lines(im, pos, a, self.ptrname)
            if a in self.labels and a != a0:
                if a in self.global_labels:
                    L.append((a, f"{self.labels[a]}:", None))
                L.append((a, f".L{a:08x}:", "local"))
            if sz < 0:
                L.append((a, inst_directive(im.bytes(a, -sz), th), -sz))
                pos = a - sz
                continue
            ins = disasm_one(im, a, th)
            L.append((a, self.ins_text(a, ins, th, a0), sz))
            pos = a + sz
        if pos < e:
            L += data_lines(im, pos, e, self.ptrname)
        return L

    def header(self, u):
        a0, e, kind, obj = u
        th = kind == "func" and obj["mode"] == "thumb"
        lines = ["    .syntax unified", "    .thumb" if th else "    .arm",
                 f'    .section .text.{a0:08x}, "ax", %progbits']
        if kind == "func":
            n = self.im.fname[a0]
            lines += [f"    .global {n}", f"    .type {n}, %function"]
            if th:
                lines.append("    .thumb_func")
            lines.append(f"{n}:  /* 0x{a0:08x} */")
            lines.append(f".Lstart_{a0:08x}:")
        elif kind == "vectors":
            lines += ["    .global vectors", "vectors:"]
        else:
            n = f"{obj}_{a0:08x}"
            lines += [f"    .global {n}", f"{n}:"]
        return lines

    def render(self, u, with_addr_comments=True):
        out = self.header(u)
        # global interior labels defined in this unit
        for a, t, s in self.unit_lines(u):
            if s is None:
                out.append(f"    .global {t[:-1]}")
                out.append(t)
            elif s == "local":
                out.append(t)
            else:
                out.append(f"    {t}" + (f"  /* {a:08x} */" if with_addr_comments else ""))
        if u[2] == "func":
            n = self.im.fname[u[0]]
            out.append(f"    .size {n}, . - {n}")
        return out


# ------------------------------------------------------------------ verification
LIST_RE = re.compile(r"^\s*(\d+)\s+([0-9a-f]{4}) ([0-9A-F]+)\s")


def verify(im, em, units, workdir):
    """Assemble all units in one file, one section per unit (same relocation behaviour as
    separate objects). Return the instruction addresses that do not reassemble identically:
    size must always match; bytes must match unless the field carries a relocation."""
    workdir.mkdir(parents=True, exist_ok=True)
    src = []
    linemap = {}   # source line -> (addr, size, unit_start)
    for u in units:
        if u[2] == "blob":
            continue                      # opaque .incbin, nothing to verify
        src += em.header(u)
        for a, t, s in em.unit_lines(u):
            if s is None or s == "local":
                src.append(t)
                continue
            src.append(f"    {t}")
            if not t.startswith((".word", ".space", ".byte", ".inst")):
                linemap[len(src)] = (a, s, u[0])
    p = workdir / "verify.s"
    p.write_text("\n".join(src) + "\n")
    r = subprocess.run([AS, *ASFLAGS, "-al=" + str(workdir / "verify.lst"), "-o", str(workdir / "verify.o"),
                        str(p)], capture_output=True, text=True)
    bad = set()
    for line in r.stderr.splitlines():
        m = re.match(r".*verify\.s:(\d+): Error", line)
        if m and int(m.group(1)) in linemap:
            bad.add(linemap[int(m.group(1))][0])
    if bad or r.returncode != 0:
        if not bad:
            sys.exit("assembler failed:\n" + r.stderr[:4000])
        return bad, False
    # relocated offsets per unit section
    rel = subprocess.run(["arm-none-eabi-readelf", "-rW", str(workdir / "verify.o")],
                         capture_output=True, text=True).stdout
    relocs, cur = {}, None
    for line in rel.splitlines():
        m = re.match(r"Relocation section '\.rel\.text\.([0-9a-f]{8})'", line)
        if m:
            cur = int(m.group(1), 16); relocs[cur] = set(); continue
        m = re.match(r"^([0-9a-f]{8})\s", line)
        if m and cur is not None:
            relocs[cur].add(int(m.group(1), 16))
    for line in (workdir / "verify.lst").read_text(errors="replace").splitlines():
        m = LIST_RE.match(line)
        if not m:
            continue
        ln = int(m.group(1))
        if ln not in linemap:
            continue
        a, s, u0 = linemap[ln]
        off = int(m.group(2), 16)
        got = bytes.fromhex(m.group(3))
        if len(got) != s or off != a - u0:
            bad.add(a)                    # size/placement change
        elif off not in relocs.get(u0, ()) and got != im.bytes(a, s):
            bad.add(a)
    return bad, True


def cmd_gen():
    im = Image()
    units = build_units(im)
    forced_path = ROOT / "build/split/forced.json"
    forced = set(json.loads(forced_path.read_text())) if forced_path.exists() else set()
    for it in range(30):
        em = Emitter(im, units, forced)
        bad, assembled = verify(im, em, units, ROOT / "build/split")
        print(f"pass {it}: {len(bad)} mismatching instructions ({'listing' if assembled else 'errors'})")
        if not bad:
            break
        forced |= bad
        forced_path.parent.mkdir(parents=True, exist_ok=True)
        forced_path.write_text(json.dumps(sorted(forced)))
    else:
        sys.exit("did not converge")
    em = Emitter(im, units, forced)
    # write outputs
    asm = ROOT / "asm"
    for old in asm.glob("*.s"):
        old.unlink()
    order = []
    blobs = []
    for u in units:
        if u[2] == "blob":
            name = f"data_{u[0]:08x}"
            blobs.append((f"{u[3]}_{u[0]:08x}", u[0], u[1],
                          hashlib.sha256(im.bytes(u[0], u[1] - u[0])).hexdigest()))
        elif u[2] == "vectors":
            name = "vectors"
        else:
            name = f"func_{u[0]:08x}"
        fn = f"{name}.s"
        body = [f"/* 0x{u[0]:08x}-0x{u[1]:08x}  generated by tools/split.py; edit freely once"
                f" it is yours */"] + em.render(u) + [""]
        (asm / fn).write_text("\n".join(body))
        order.append((u[0], fn))
    with open(ROOT / "data/blobs.csv", "w", newline="") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["name", "start", "end", "sha256"])
        for n, a, e, h in blobs:
            w.writerow([n, f"0x{a:08X}", f"0x{e:08X}", h])
    lines = ["/* generated by tools/split.py: unit sections in original address order */"]
    for a, fn in order:
        lines.append(f"        KEEP(*(.text.{a:08x}))  /* {fn} */")
    (ROOT / "ld/sections.ld").write_text("\n".join(lines) + "\n")
    (ROOT / "asm/ORDER").write_text("".join(f"{fn}\n" for _, fn in order))
    n_inst = len(forced)
    n_ins = sum(1 for u in units if u[2] == "func" for _ in em._linear(u[3]))
    print(f"units={len(units)} files={len(order)} blobs={len(blobs)} insns={n_ins} forced_inst={n_inst}")
    if em.mode_conflicts:
        print("mode conflicts (branch to a function of the other ISA; emitted raw):",
              ", ".join(f"0x{a:08x}->0x{t:08x}" for a, t in sorted(em.mode_conflicts)))


def cmd_extract():
    img = (ROOT / "build/extract/app.bin").read_bytes()
    out = ROOT / "data"
    for r in csv.DictReader(open(out / "blobs.csv")):
        a, e = int(r["start"], 16), int(r["end"], 16)
        b = img[a - BASE:e - BASE]
        if hashlib.sha256(b).hexdigest() != r["sha256"]:
            sys.exit(f"blob {r['name']}: hash mismatch (different firmware build?)")
        (out / f"{r['name']}.bin").write_bytes(b)
    print("data blobs extracted")


if __name__ == "__main__":
    {"gen": cmd_gen, "extract": cmd_extract}[sys.argv[1]]()
