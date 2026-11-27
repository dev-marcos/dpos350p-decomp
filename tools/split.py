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
# End of the part of the image that may move. From here on the file overlaps the ZI (.bss)
# region, whose base 0x400954A4 is fixed by the scatter-load table; a word with a value at or
# above it is a RAM address, never a reference to something that moves with the code.
RELOC_END = 0x400954A4
# Words that hold the offset from their own address to a function: the armlink Region$$Table
# (docs/MEMORY_MAP.md) starts each entry with the handler's offset from the entry, which
# __scatterload adds to the entry address. One entry: __scatterload_zeroinit.
REL_WORDS = [0x4008D9F0]
# Pointer words that no decoded instruction loads (so the analyser did not mark them as literal
# pool entries) but that hold an exact image address: 0x4008CCE0, right after tanh's ten
# coefficients, is the address of exp's table 0x4008C898. Emitted as symbols so that they follow
# their target if anything does read them.
EXTRA_POINTERS = [0x4008CCE0]

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
        for a in EXTRA_POINTERS:
            self.lit.setdefault(a, 4)

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


_DIS = {}


def disasm_one(im, a, thumb):
    # The instruction's operand details live in a buffer that capstone frees when its
    # generator is collected: keep the generator (cached per address) so they stay valid.
    k = (a, thumb)
    if k not in _DIS:
        g = MD[thumb].disasm(im.bytes(a, 4), a, 1)
        _DIS[k] = (g, next(g, None))
    return _DIS[k][1]


def inst_directive(ins_bytes, thumb):
    if not thumb:
        return f".inst 0x{struct.unpack('<I', ins_bytes)[0]:08x}"
    if len(ins_bytes) == 2:
        return f".inst.n 0x{struct.unpack('<H', ins_bytes)[0]:04x}"
    hi, lo = struct.unpack("<HH", ins_bytes)
    return f".inst.w 0x{hi:04x}{lo:04x}"


LOAD_RE = re.compile(r"\[pc(?:, #(-?0x[0-9a-f]+|-?\d+))?\]$")
ADDPC_RE = re.compile(r"^(add|sub|adr)(w|eq|ne|cs|hs|cc|lo|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al)?(\.w)?$")


C_REF_RE = re.compile(r"\b(?:IMG|IMGU|IMG_DECL|STR|S)\(([0-9a-f]{8})\)")


def c_image_refs():
    """Image addresses the C sources name as d_XXXXXXXX (include/image.h): each needs a label."""
    refs = set()
    for p in list((ROOT / "nonmatching").rglob("*.c")) + list((ROOT / "include").rglob("*.h")):
        refs.update(int(m, 16) for m in C_REF_RE.findall(p.read_text(encoding="utf-8", errors="replace")))
    return refs


def pcrel_target(ins, a, th):
    """Address an instruction reads relative to the PC: 'ldr* / vldr rX, [pc, #imm]' or
    'add / sub / adr rX, pc, #imm'. None for anything else (pc in a register list,
    [pc, rX] table lookups, writes to pc...)."""
    pcv = ((a + 4) & ~3) if th else a + 8
    m = LOAD_RE.search(ins.op_str)
    if ins.mnemonic.startswith(("ldr", "vldr")) and m:
        return pcv + (int(m.group(1), 0) if m.group(1) else 0)
    m = ADDPC_RE.match(ins.mnemonic)
    ops = ins.operands
    if m and len(ops) >= 2 and ops[0].type == A.ARM_OP_REG and ops[0].reg != A.ARM_REG_PC \
            and ops[-1].type == A.ARM_OP_IMM:
        if m.group(1) == "adr" and len(ops) == 2:
            return pcv + ops[1].imm
        if len(ops) == 3 and ops[1].type == A.ARM_OP_REG and ops[1].reg == A.ARM_REG_PC:
            return pcv - ops[2].imm if m.group(1) == "sub" else pcv + ops[2].imm
    return None


def is_terminal(ins):
    """Unconditional transfer: execution never runs on into the next instruction."""
    if ins.cc not in (A.ARM_CC_AL, A.ARM_CC_INVALID):
        return False
    if ins.id in (A.ARM_INS_B, A.ARM_INS_BX):
        return True
    ops = ins.op_str
    if ins.mnemonic.startswith(("pop", "ldm")) and "pc" in ops:
        return True
    return bool(ins.operands) and ins.operands[0].type == A.ARM_OP_REG and \
        ins.operands[0].reg == A.ARM_REG_PC and not ins.mnemonic.startswith(("cmp", "str", "tst"))


def data_lines(im, a, e, ptrname, cuts=frozenset()):
    """Bytes [a,e) that are not instructions: pool words (maybe symbolised), zeros, bytes.
    No line crosses an address in 'cuts' (a label is placed there)."""
    out = []
    x = a
    while x < e:
        n = (im.lit[x] if x + im.lit[x] <= e else 4) if x in im.lit else 0
        if n >= 4 and x + 4 <= e and (x & 3) == 0 and \
                not any(x + k < c < x + k + 4 for k in range(0, n, 4) for c in cuts):
            for k in range(0, n, 4):
                v = im.u32(x + k)
                # an 8-byte literal is a double (vldr d / ldrd): its words are never addresses
                out.append((x + k, f".word {ptrname(v) if im.lit[x] != 8 else f'0x{v:08x}'}", 4))
            x += max(4, min(n, e - x)) // 4 * 4
            continue
        z = x
        while z < e and im.img[z - BASE] == 0 and z not in im.lit and (z == x or z not in cuts):
            z += 1
        if z - x >= 4:
            out.append((x, f".space {z - x}", z - x))
            x = z
            continue
        y = x + 1
        while y < e and (y - x) < 16 and y not in im.lit and y not in cuts and \
                not (im.img[y - BASE] == 0 and im.img[y - BASE:y - BASE + 4] == b"\0\0\0\0"):
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
        self.blob_of = [(u[0], u[1], f"{u[3]}_{u[0]:08x}") for u in units if u[2] == "blob"]
        self._scan_references()
        self.clusters = self._clusters()

    # ---- references that must follow their target when the code moves (relocatable output)
    def _scan_references(self):
        """PC-relative operands become local labels, image addresses in literal pools and in
        movw/movt pairs become symbols: then a cluster (see _clusters) can be linked anywhere."""
        im = self.im
        self.pcrel = {}            # insn addr -> target addr (literal loads, add/sub rX, pc, #imm)
        self.movsym = {}           # insn addr -> (':lower16:' | ':upper16:', target addr)
        for u in self.units:
            if u[2] != "func":
                continue
            f = u[3]
            th = f["mode"] == "thumb"
            lin = [(a, s) for a, s in self._linear(f) if s > 0]
            for i, (a, sz) in enumerate(lin):
                ins = disasm_one(im, a, th)
                if "pc" in ins.op_str and ins.id not in BR_IDS:
                    t = pcrel_target(ins, a, th)
                    if t is not None:
                        self.pcrel[a] = t
                if ins.mnemonic.startswith("movt") and ins.operands[1].type == A.ARM_OP_IMM:
                    # read the operands now: decoding another instruction reuses capstone's buffer
                    rd, hi = ins.operands[0].reg, ins.operands[1].imm
                    for b, _ in reversed(lin[max(0, i - 6):i]):
                        w = disasm_one(im, b, th)
                        if w.mnemonic.startswith("movw") and w.operands[0].reg == rd:   # capstone ids movw as MOV
                            v = (hi << 16) | w.operands[1].imm
                            # 0x40000000 from movw #0 / movt #0x4000 is the high word of the
                            # double 2.0 (pow), never the image base
                            if BASE < v < RELOC_END:
                                self.movsym[b] = (":lower16:", v, None)
                                self.movsym[a] = (":upper16:", v, None)
                            else:
                                self._mov_pcrel(lin, i, rd, v, b, a, th)
                            break
                        if w.operands and w.operands[0].type == A.ARM_OP_REG and w.operands[0].reg == rd:
                            break            # rd written by something else first: no pair
        self.pc_targets = set(self.pcrel.values())
        # addresses that need a global symbol (not function starts, which have one already)
        self.data_refs = set()
        for la, sz in im.lit.items():
            if sz == 8:
                continue
            for k in range(0, sz, 4):
                v = im.u32(la + k)
                if BASE <= v < RELOC_END and self.func_sym(v) is None:
                    self.data_refs.add(v)
        for _, v, _ in self.movsym.values():
            if self.func_sym(v) is None:
                self.data_refs.add(v)
        # words that hold an offset from their own address (see REL_WORDS)
        self.relwords = {}
        for a in REL_WORDS:
            v = (a + struct.unpack("<i", im.bytes(a, 4))[0]) & 0xFFFFFFFF
            assert self.func_sym(v), f"0x{a:08x}: relative word does not reach a function"
            self.relwords[a] = v
        # addresses the C names through include/image.h (IMG, IMGU, IMG_DECL, STR, S)
        self.c_refs = c_image_refs()
        self.data_refs |= {v for v in self.c_refs if self.func_sym(v) is None}
        self.data_refs.discard(BASE)          # 'vectors'

    def _mov_pcrel(self, lin, i, rd, k16, pw, pt, th):
        """movw/movt rd, K ; add rd, pc [; sub rd, rd, #k]: position-independent address
        (armlink __scatterload, libm tables). Becomes movw/movt with a PC-relative relocation:
        (S + A - P) is K at both P when A = P - anchor + k."""
        im = self.im
        nxt = lin[i + 1:i + 3]
        if not nxt:
            return
        add_a = nxt[0][0]
        ins = disasm_one(im, add_a, th)
        ops = ins.operands
        if not (ins.mnemonic == "add" and len(ops) == 2 and ops[0].reg == rd and
                ops[1].type == A.ARM_OP_REG and ops[1].reg == A.ARM_REG_PC):
            return
        anchor = add_a + (4 if th else 8)
        k = 0
        if len(nxt) > 1:
            s = disasm_one(im, nxt[1][0], th)
            so = s.operands
            if s.mnemonic.startswith("sub") and len(so) == 3 and so[0].reg == rd and \
                    so[1].type == A.ARM_OP_REG and so[1].reg == rd and so[2].type == A.ARM_OP_IMM:
                k = so[2].imm
        t = (anchor + k16 - k) & 0xFFFFFFFF
        if BASE <= t < RELOC_END:
            self.movsym[pw] = (":lower16:", t, pw - anchor + k)
            self.movsym[pt] = (":upper16:", t, pt - anchor + k)

    def func_sym(self, v):
        im = self.im
        if v in im.funcs and im.funcs[v]["mode"] == "arm":
            return im.fname[v]
        if v & 1 and (v - 1) in im.funcs and im.funcs[v - 1]["mode"] == "thumb":
            return im.fname[v - 1]
        return None

    def data_sym(self, v):
        """Symbol for an image address that is not a function entry: d_XXXXXXXX, the same name
        the C uses for it (include/image.h)."""
        return "vectors" if v == BASE else f"d_{v:08x}"

    def _clusters(self):
        """Contiguous runs of units that must stay together: a PC-relative operand in one unit
        reaches into another (armcc shares one literal pool per section), or a function runs
        on into the next one. Each cluster is one section; clusters can be placed anywhere."""
        starts = [u[0] for u in self.units]
        import bisect
        idx = lambda a: bisect.bisect_right(starts, a) - 1   # noqa: E731
        span = {}                  # unit index -> furthest unit index it must be joined to
        def need(i, j):
            lo, hi = min(i, j), max(i, j)
            span[lo] = max(span.get(lo, lo), hi)
        for a, t in self.pcrel.items():
            need(idx(a), idx(t))
        # a branch kept as raw .inst (Thumb 16-bit B/B<c>/CBZ to another unit, forced
        # encodings) has a fixed offset: its target must stay at the same distance
        for i, u in enumerate(self.units):
            if u[2] != "func":
                continue
            th = u[3]["mode"] == "thumb"
            for a, sz in self._linear(u[3]):
                if sz < 0:
                    continue
                ins = disasm_one(self.im, a, th)
                if ins.id in BR_IDS and ins.operands and ins.operands[-1].type == A.ARM_OP_IMM:
                    t = ins.operands[-1].imm
                    if BASE <= t < BASE + len(self.im.img) and idx(t) != i and \
                            self.ins_text(a, ins, th, u[0]).startswith(".inst"):
                        need(i, idx(t))
        for i, u in enumerate(self.units):
            if u[2] != "func" or i + 1 >= len(self.units) or self.units[i + 1][2] != "func":
                continue
            lin = [(a, s) for a, s in self._linear(u[3]) if s > 0]
            if not lin or lin[-1][0] + lin[-1][1] != u[1]:
                continue                     # data after the last instruction: no fall-through
            ins = disasm_one(self.im, lin[-1][0], u[3]["mode"] == "thumb")
            if not is_terminal(ins):
                need(i, i + 1)
        out, i = [], 0
        while i < len(self.units):
            j, hi = i, span.get(i, i)
            while j < hi:
                j += 1
                hi = max(hi, span.get(j, j))
            out.append((i, hi))
            i = hi + 1
        return out                           # [(first unit index, last unit index)]

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
        s = self.func_sym(v)
        if s is not None:
            return s
        if BASE <= v < RELOC_END:
            return self.data_sym(v)
        return f"0x{v:08x}"

    def ins_text(self, a, ins, th, here=None):
        if a in self.forced:
            return inst_directive(ins.bytes, th)
        text = f"{ins.mnemonic} {ins.op_str}".strip()
        if a in self.pcrel:
            lab = f".Lp_{self.pcrel[a]:08x}"
            if LOAD_RE.search(ins.op_str):
                return f"{ins.mnemonic} {LOAD_RE.sub(lab, ins.op_str)}"
            m = ADDPC_RE.match(ins.mnemonic)
            cond = m.group(2) if m.group(2) not in (None, "w") else ""
            wide = ".w" if (m.group(2) == "w" or m.group(3)) else ""
            return f"adr{cond}{wide} {ins.op_str.split(',')[0]}, {lab}"
        if a in self.movsym:
            part, v, add = self.movsym[a]
            expr = self.ptrname(v) if add is None else f"({self.ptrname(v)} - . + {add})"
            return re.sub(r"#0x[0-9a-f]+$|#\d+$", f"#{part}{expr}", text)
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
        L = self._unit_lines(u)
        a0, e = u[0], u[1]
        if u[2] == "blob":
            return L                         # labels placed by _unit_lines
        if not hasattr(self, "dlabels"):
            self.dlabels = {v for v in self.data_refs if self.data_sym(v).startswith("d_")}
        want = {x for x in self.pc_targets | self.dlabels if a0 <= x < e}
        if not want:
            return L
        out, done = [], set()
        for a, t, s in L:
            if isinstance(s, int) and a in want and a not in done:
                if a in self.dlabels:
                    out.append((a, f"d_{a:08x}:", None))
                if a in self.pc_targets:
                    out.append((a, f".Lp_{a:08x}:", "local"))
                done.add(a)
            out.append((a, t, s))
        missing = want - done
        assert not missing, f"unit 0x{a0:08x}: no line starts at {sorted(hex(x) for x in missing)}"
        return out

    def _unit_lines(self, u):
        im = self.im
        a0, e, kind, obj = u
        L = []
        rel = {x for x in self.relwords if a0 <= x < e}
        cuts = frozenset(x for x in self.pc_targets | self.data_refs | rel | {x + 4 for x in rel}
                         if a0 < x < e)
        if kind == "blob":
            name = f"{obj}_{a0:08x}"
            pts = [a0] + sorted(cuts) + [e]
            for lo, hi in zip(pts, pts[1:]):
                if lo in self.data_refs:
                    L.append((lo, f"d_{lo:08x}:", None))
                if lo in rel:
                    assert hi == lo + 4
                    L.append((lo, f".word {self.ptrname(self.relwords[lo])} - .", 4))
                else:
                    L.append((lo, f'.incbin "data/{name}.bin", {lo - a0}, {hi - lo}', hi - lo))
            return L
        if kind == "vectors":
            for k in range(8):
                ins = disasm_one(im, a0 + 4 * k, False)
                L.append((a0 + 4 * k, self.ins_text(a0 + 4 * k, ins, False), 4))
            for k in range(8):
                L.append((a0 + 0x20 + 4 * k, f".word {self.ptrname(im.u32(a0 + 0x20 + 4 * k))}", 4))
            if e > a0 + 0x40:
                L += [(x, t, s) for x, t, s in data_lines(im, a0 + 0x40, e, self.ptrname, cuts)]
            return L
        f = obj
        th = f["mode"] == "thumb"
        lin = self._linear(f)
        pos = a0
        for a, sz in lin:
            if a > pos:
                L += data_lines(im, pos, a, self.ptrname, cuts)
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
            L += data_lines(im, pos, e, self.ptrname, cuts)
        return L

    def header(self, u):
        a0, e, kind, obj = u
        th = kind == "func" and obj["mode"] == "thumb"
        # no .section here: the cluster file (asm/sections/) that .include's the unit sets it
        lines = ["    .syntax unified", "    .thumb" if th else "    .arm"]
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
    """Assemble all units in one file, one section per cluster (same relocation behaviour as
    separate objects). Return the instruction addresses that do not reassemble identically:
    size must always match; bytes must match unless the field carries a relocation."""
    workdir.mkdir(parents=True, exist_ok=True)
    src = []
    linemap = {}   # source line -> (addr, size, cluster_start)
    for i, j in em.clusters:
        c0 = units[i][0]
        if units[i][2] == "blob":
            continue                      # opaque .incbin, nothing to verify
        src.append(f'    .section .text.{c0:08x}, "ax", %progbits')
        for u in units[i:j + 1]:
            src += em.header(u)
            for a, t, s in em.unit_lines(u):
                if s is None or s == "local":
                    if s is None:
                        src.append(f"    .global {t[:-1]}")
                    src.append(t)
                    continue
                src.append(f"    {t}")
                if not t.startswith((".word", ".space", ".byte", ".inst")):
                    linemap[len(src)] = (a, s, c0)
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
    secdir = asm / "sections"
    secdir.mkdir(exist_ok=True)
    for old in secdir.glob("*.s"):
        old.unlink()
    fname = {}
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
                f" it is yours. Assembled through asm/sections/ (one section per cluster). */"] + \
            em.render(u) + [""]
        (asm / fn).write_text("\n".join(body))
        fname[u[0]] = fn
    secs = []
    for i, j in em.clusters:
        c0, c1 = units[i][0], units[j][1]
        body = [f"/* 0x{c0:08x}-0x{c1:08x}  generated by tools/split.py. One cluster: units that",
                " * must stay together (shared literal pools, PC-relative operands, fall-through).",
                " * The cluster is one section and the linker may place it anywhere. */",
                f'    .section .text.{c0:08x}, "ax", %progbits']
        body += [f'    .include "asm/{fname[u[0]]}"' for u in units[i:j + 1]]
        sn = f"sec_{c0:08x}.s"
        (secdir / sn).write_text("\n".join(body) + "\n")
        secs.append((c0, sn, [fname[u[0]] for u in units[i:j + 1]]))
    with open(ROOT / "data/blobs.csv", "w", newline="") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["name", "start", "end", "sha256"])
        for n, a, e, h in blobs:
            w.writerow([n, f"0x{a:08X}", f"0x{e:08X}", h])
    lines = ["/* generated by tools/split.py: cluster sections in original address order */"]
    for a, sn, fns in secs:
        lines.append(f"        KEEP(*(.text.{a:08x}))  /* {sn}: {', '.join(fns[:3])}"
                     f"{' ...' if len(fns) > 3 else ''} */")
    (ROOT / "ld/sections.ld").write_text("\n".join(lines) + "\n")
    (ROOT / "asm/ORDER").write_text("".join(f"{fn}\n" for _, _, fns in secs for fn in fns))
    n_pc = len(em.pcrel)
    n_pc_raw = sum(1 for a in em.pcrel if a in forced)
    print(f"clusters={len(secs)} pcrel={n_pc} (raw .inst: {n_pc_raw}) "
          f"data symbols={len(em.data_refs)} movw/movt={len(em.movsym)}")
    for a, (part, v, add) in sorted(em.movsym.items()):
        print(f"  movw/movt 0x{a:08x} {part}{em.ptrname(v)}" + ("" if add is None else f" (PC-relative, addend {add})"))
    for a, v in sorted(em.relwords.items()):
        print(f"  relative word 0x{a:08x} -> {em.ptrname(v)}")
    n_inst = len(forced)
    n_ins = sum(1 for u in units if u[2] == "func" for _ in em._linear(u[3]))
    print(f"units={len(units)} files={len(fname)} blobs={len(blobs)} insns={n_ins} forced_inst={n_inst}")
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
