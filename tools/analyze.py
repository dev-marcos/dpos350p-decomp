#!/usr/bin/env python3
"""Static map of the DPOS350P application image (build 2025-06-19).

Recursive-descent disassembly (Capstone) seeded by:
  * the REA/Ghidra procedure inventory  (evidence/phase2/rea-list_procedures.json)
  * the exception vectors and the startup code
  * every direct call target discovered on the way (BL -> ARM, BLX imm -> Thumb)

For each function it records the instruction bytes, the PC-relative literal-pool words
it loads, and jump tables, which gives exact [start, end) extents including the pool.
Every byte of the image is then classified as function / data / padding, and the
uncertain parts are marked OPEN.

Output: build/analysis/map.json (consumed by tools/mkmap.py and later tools/split.py)
"""
import json
import struct
import sys
from bisect import bisect_right
from pathlib import Path

from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB
from capstone import arm as A

ROOT = Path(__file__).resolve().parent.parent
BASE = 0x40000000

IMG = (ROOT / "build/extract/app.bin").read_bytes()
# Load region ends where the zero-init region starts (scatter table at 0x4008D9F0):
# bytes past it are Windows stack garbage left by FNIRSI's packer (see MEMORY_MAP.md).
LOAD_END = 0x400954A4
END = BASE + len(IMG)

md_arm = Cs(CS_ARCH_ARM, CS_MODE_ARM); md_arm.detail = True
md_thumb = Cs(CS_ARCH_ARM, CS_MODE_THUMB); md_thumb.detail = True

def u32(a):
    return struct.unpack_from("<I", IMG, a - BASE)[0]

def in_image(a):
    return BASE <= a < LOAD_END

_cache = {}
def decode(addr, thumb):
    key = (addr, thumb)
    if key not in _cache:
        md = md_thumb if thumb else md_arm
        off = addr - BASE
        ins = next(md.disasm(IMG[off:off + 4], addr, 1), None)
        _cache[key] = ins
    return _cache[key]

BRANCH_IDS = {A.ARM_INS_B, A.ARM_INS_BL, A.ARM_INS_BLX, A.ARM_INS_BX, A.ARM_INS_CBZ, A.ARM_INS_CBNZ}
LIT_SIZES = {A.ARM_INS_LDR: 4, A.ARM_INS_LDRB: 1, A.ARM_INS_LDRSB: 1, A.ARM_INS_LDRH: 2,
             A.ARM_INS_LDRSH: 2, A.ARM_INS_LDRD: 8, A.ARM_INS_VLDR: None}

def writes_pc(ins):
    if ins.id in (A.ARM_INS_POP, A.ARM_INS_LDM, A.ARM_INS_LDMIB, A.ARM_INS_LDMDA, A.ARM_INS_LDMDB):
        return any(op.type == A.ARM_OP_REG and op.reg == A.ARM_REG_PC for op in ins.operands)
    ops = ins.operands
    return bool(ops) and ops[0].type == A.ARM_OP_REG and ops[0].reg == A.ARM_REG_PC and \
        ins.id not in (A.ARM_INS_CMP, A.ARM_INS_CMN, A.ARM_INS_TST, A.ARM_INS_TEQ,
                       A.ARM_INS_STR, A.ARM_INS_STM, A.ARM_INS_PUSH, A.ARM_INS_BX, A.ARM_INS_BLX)

def cond(ins):
    return ins.cc not in (A.ARM_CC_AL, A.ARM_CC_INVALID)

def find_bound(insns_back, reg):
    """Look back for 'cmp reg, #imm' -> number of table entries (imm+1)."""
    for p in insns_back[::-1][:6]:
        if p.id == A.ARM_INS_CMP and len(p.operands) == 2 and p.operands[0].type == A.ARM_OP_REG \
                and p.operands[0].reg == reg and p.operands[1].type == A.ARM_OP_IMM:
            return p.operands[1].imm + 1
    return None


class Func:
    def __init__(self, start, thumb):
        self.start, self.thumb = start, thumb
        self.insns = {}        # addr -> size
        self.literals = {}     # addr -> size
        self.tables = []       # (addr, size, kind)
        self.calls = set()     # (target, thumb)
        self.tailcalls = set()
        self.notes = []
        self.end = start

    def to_json(self):
        return {"start": self.start, "end": self.end, "mode": "thumb" if self.thumb else "arm",
                "origin": getattr(self, "origin", "?"),
                "n_insns": len(self.insns), "literals": sorted(self.literals.items()),
                "tables": self.tables, "calls": sorted(t for t, _ in self.calls),
                "tailcalls": sorted(self.tailcalls), "notes": self.notes,
                "holes": getattr(self, "holes", [])}


def analyse_function(start, thumb, stops, limit=None):
    """Follow all intra-procedural flow from start. 'stops' = known function entries:
    a branch to one of them is a tail call, falling into one ends the walk."""
    f = Func(start, thumb)
    work = [start]
    trail = {}   # addr -> list of preceding insns in straight line (for bound search)
    while work:
        a = work.pop()
        prev = trail.get(a, [])
        while True:
            if a in f.insns or a in f.literals:
                break
            if not in_image(a) or not in_code_span(a):
                f.notes.append(f"flow leaves code at 0x{a:08x}")
                break
            if a != start and a in stops:
                f.notes.append(f"falls into 0x{a:08x}")
                break
            ins = decode(a, thumb)
            if ins is None:
                f.notes.append(f"undecodable at 0x{a:08x}")
                break
            f.insns[a] = ins.size
            nxt = a + ins.size
            pc = (a + 4) & ~3 if thumb else a + 8
            # ---- literal loads
            if ins.id in LIT_SIZES:
                for op in ins.operands:
                    if op.type == A.ARM_OP_MEM and op.mem.base == A.ARM_REG_PC and op.mem.index == 0:
                        disp = op.mem.disp
                        if getattr(op, "subtracted", False) and disp > 0:
                            disp = -disp
                        la = (pc if not thumb else pc) + disp
                        sz = LIT_SIZES[ins.id]
                        if sz is None:
                            sz = 8 if ins.operands[0].reg >= A.ARM_REG_D0 and ins.operands[0].reg <= A.ARM_REG_D31 else 4
                        if in_image(la):
                            f.literals[la] = max(f.literals.get(la, 0), sz)
            stop = False
            if ins.id in BRANCH_IDS:
                op = ins.operands[-1]
                if ins.id in (A.ARM_INS_CBZ, A.ARM_INS_CBNZ):
                    work.append(op.imm); trail[op.imm] = prev[-6:] + [ins]
                elif ins.id == A.ARM_INS_B:
                    t = op.imm
                    if t in stops and t != start or not in_code_span(t):
                        f.tailcalls.add(t)
                    else:
                        work.append(t); trail[t] = prev[-6:] + [ins]
                    stop = not cond(ins)
                elif ins.id == A.ARM_INS_BL and op.type == A.ARM_OP_IMM:
                    f.calls.add((op.imm, thumb))
                elif ins.id == A.ARM_INS_BLX and op.type == A.ARM_OP_IMM:
                    f.calls.add((op.imm, not thumb))
                elif ins.id == A.ARM_INS_BX:
                    stop = not cond(ins)
                # BLX reg: indirect call, continue
            elif ins.id in (A.ARM_INS_TBB, A.ARM_INS_TBH):
                mem = ins.operands[0].mem
                n = find_bound(prev, mem.index)
                esz = 1 if ins.id == A.ARM_INS_TBB else 2
                if n is None:
                    f.notes.append(f"tb* without bound at 0x{a:08x}")
                else:
                    tb = nxt
                    f.tables.append((tb, n * esz, "tbb" if esz == 1 else "tbh"))
                    for i in range(n):
                        e = IMG[tb - BASE + i * esz] if esz == 1 else struct.unpack_from("<H", IMG, tb - BASE + 2 * i)[0]
                        t = tb + 2 * e
                        work.append(t); trail[t] = []
                    for i in range(0, (n * esz + 1) & ~1, 2):
                        f.literals[tb + i] = 2
                stop = True
            elif writes_pc(ins):
                ops = ins.operands
                # armcc:  addls pc, pc, rX, lsl #2 ; b default ; b case0 ...
                if ins.id == A.ARM_INS_ADD and len(ops) == 3 and ops[1].type == A.ARM_OP_REG and \
                        ops[1].reg == A.ARM_REG_PC and ops[2].type == A.ARM_OP_REG:
                    n = find_bound(prev, ops[2].reg)
                    if n is None:
                        f.notes.append(f"add-pc jump table without bound at 0x{a:08x}")
                    else:
                        f.tables.append((a + 8, 4 * n, "arm-branch"))
                        for i in range(n):
                            work.append(a + 8 + 4 * i); trail[a + 8 + 4 * i] = []
                    stop = not cond(ins)
                # gcc:  ldrls pc, [pc, rX, lsl #2] ; b default ; .word case...
                elif ins.id == A.ARM_INS_LDR and ops[1].type == A.ARM_OP_MEM and \
                        ops[1].mem.base == A.ARM_REG_PC and ops[1].mem.index != 0:
                    n = find_bound(prev, ops[1].mem.index)
                    if n is None:
                        f.notes.append(f"ldr-pc jump table without bound at 0x{a:08x}")
                    else:
                        tb = a + 8
                        f.tables.append((tb, 4 * n, "word"))
                        for i in range(n):
                            f.literals[tb + 4 * i] = 4
                            t = u32(tb + 4 * i)
                            work.append(t & ~1); trail[t & ~1] = []
                    stop = not cond(ins)
                else:
                    # long-branch veneer: ldr pc, [pc, #imm] -> the literal is the target
                    if ins.id == A.ARM_INS_LDR and ops[1].type == A.ARM_OP_MEM and \
                            ops[1].mem.base == A.ARM_REG_PC and ops[1].mem.index == 0:
                        la = pc + ops[1].mem.disp
                        if in_image(la):
                            v = u32(la)
                            f.calls.add((v & ~1, bool(v & 1)))
                    stop = not cond(ins)
            elif ins.id in (A.ARM_INS_UDF,) or (ins.id == A.ARM_INS_SVC and False):
                stop = True
            prev = prev[-6:] + [ins]
            if stop:
                break
            a = nxt
    # Code extent only: armcc shares one literal pool between consecutive functions of a
    # section, so pools are classified separately (see classify()).
    # Body = all code reachable from the entry. Holes inside (bytes jumped over, e.g. armcc
    # inline constants/strings, mid-function pools) belong to the function. A detached
    # piece that starts with a prologue is a separate function reached by a tail branch.
    code = sorted([(a, a + s) for a, s in f.insns.items()] +
                  [(t, t + n) for t, n, k in f.tables if k == "arm-branch"])
    end = start
    f.chunks = []
    limit = limit or LOAD_END
    for a, e in code:
        if a < start:
            continue
        if a >= limit:
            f.notes.append(f"reaches 0x{a:08x} beyond next entry 0x{limit:08x}")
            break
        if a > end and is_prologue(a) is not None:
            f.chunks.append(a)
            break
        end = max(end, e)
    f.holes = []
    x = start
    for a, e in code:
        if a >= end:
            break
        if a > x:
            f.holes.append((x, a))
        x = max(x, e)
    f.end = end
    return f


def load_seeds():
    rea = json.loads((ROOT / "evidence/phase2/rea-list_procedures.json").read_text())["normalized_result"]
    seeds = {}
    for p in rea:
        a = int(p["address"], 16)
        seeds[a] = None          # mode unknown yet
    return seeds


def guess_mode(a):
    if a & 2:
        return True
    ins_a = decode(a, False)
    return ins_a is None


# Code spans (from the REA inventory + inspection): main code, then startup/runtime stubs.
# Fonts sit below the first span, graphics between the spans, strings/tables above.
CODE_SPANS = [(0x40019000, 0x4006DE00),                       # main code
              (0x4008CB28, 0x4008CC4C),                       # reset, VFP enable, scatter loop, exit
              (0x4008CCE4, 0x4008CD30),                       # exception vector handlers
              (0x4008CF80, 0x4008CFB8)]                       # zero-init handler (Thumb)

def in_code_span(a):
    return any(lo <= a < hi for lo, hi in CODE_SPANS)
VECTOR_TARGETS = [0x4008CB28, 0x4008CCE4, 0x4008CCE8, 0x4008CCEC, 0x4008CCF0, 0x4008CCF4, 0x4008CD2C]
SCATTER_HANDLERS = [(0x4008CF80, True)]   # from the region table at 0x4008D9F0 (offset | 1)


def plausible(a, thumb, n=4):
    """Cheap validity check for a candidate entry: n decodable, non-zero instructions."""
    x = a
    for _ in range(n):
        if not in_image(x):
            return False
        ins = decode(x, thumb)
        if ins is None or not any(ins.bytes):
            return False
        if ins.id in BRANCH_IDS and ins.id == A.ARM_INS_BX:
            return True
        x += ins.size
    return True


def reaches_return(a, thumb, n=24):
    """Straight-line decode reaches 'bx lr' / pop-pc / unconditional branch without junk."""
    x = a
    for _ in range(n):
        if not in_image(x):
            return False
        ins = decode(x, thumb)
        if ins is None or not any(ins.bytes) or ins.id == A.ARM_INS_UDF:
            return False
        if not cond(ins) and (ins.id == A.ARM_INS_BX or writes_pc(ins) or ins.id == A.ARM_INS_B):
            return True
        x += ins.size
    return False


def unconditional_head(a, thumb, n):
    """First n instructions are unconditional (ASCII/tables decode as conditional ARM ops)."""
    x = a
    for _ in range(n):
        ins = decode(x, thumb)
        if ins is None or cond(ins) and ins.id != A.ARM_INS_B:
            return False
        x += ins.size
    return True


def shadowed_by_arm(a, thumb):
    """A Thumb candidate at word+2 whose containing word is an ARM prologue is a false hit
    (the upper halfword of ARM 'push {..,lr}' = 0xE92D looks like Thumb 'push.w')."""
    return thumb and (a & 3) == 2 and (u32(a - 2) & 0xFFFF0000) in (0xE92D0000, 0xE8BD0000)


def is_prologue(a):
    """ARM 'push {...,lr}' / Thumb 'push {...,lr}' or 'push.w {...,lr}' at a."""
    if (a & 3) == 0:
        w = u32(a)
        if w & 0xFFFF4000 == 0xE92D4000:
            return False
    h = struct.unpack_from("<H", IMG, a - BASE)[0]
    if h & 0xFF00 == 0xB500:
        return True
    if h == 0xE92D and struct.unpack_from("<H", IMG, a - BASE + 2)[0] & 0x4000:
        return True
    return None


def main():
    out_dir = ROOT / "build/analysis"
    out_dir.mkdir(parents=True, exist_ok=True)
    origin = {}   # start -> origin tag
    forced = {}   # start -> thumb?
    for a in load_seeds():
        if a != BASE:
            origin[a] = "rea"
    for a in VECTOR_TARGETS:
        origin.setdefault(a, "vector"); forced[a] = False
    for a, t in SCATTER_HANDLERS:
        origin.setdefault(a, "scatter-table"); forced[a] = t
    callers_mode = {}
    rounds = 0
    while True:
        rounds += 1
        order = sorted(origin)
        funcs = {}
        new = {}
        stops = set(order)
        for i, s_ in enumerate(order):
            nxt = order[i + 1] if i + 1 < len(order) else LOAD_END
            if s_ in forced:
                thumb = forced[s_]
            else:
                m = callers_mode.get(s_)
                thumb = (True in m and False not in m) if m else guess_mode(s_)
            f = analyse_function(s_, thumb, stops, nxt)
            funcs[s_] = f
            for t, tm in f.calls:
                callers_mode.setdefault(t, set()).add(tm)
                if in_image(t) and t not in origin:
                    new[t] = "call"
                    if tm:
                        forced[t] = True
            for c in f.chunks:
                # first address of a detached chunk: a function reached only by a tail call
                if c not in origin and c > s_:
                    new.setdefault(c, "chunk")
                    forced.setdefault(c, f.thumb)
        # covered code bytes
        covered = bytearray(len(IMG))
        lit = set()
        for f in funcs.values():
            for ia, sz in f.insns.items():
                if not (f.start <= ia < f.end):
                    continue
                covered[ia - BASE:ia - BASE + sz] = b"\x01" * sz
            for la, sz in f.literals.items():
                lit.update(range(la, la + sz))
        def free(x):
            return not covered[x - BASE] and x not in lit
        # pointer scan: words anywhere in the image that point into uncovered code area
        for off in range(0, LOAD_END - BASE - 3, 4):
            v = struct.unpack_from("<I", IMG, off)[0]
            t, th = (v & ~1, True) if v & 1 else (v, False)
            if not in_code_span(t) or (not th and t & 3) or t in origin or t in new:
                continue
            if not free(t) or not plausible(t, th, 6) or shadowed_by_arm(t, th):
                continue
            # Thumb pointers need a real Thumb prologue; ARM ones a prologue or a short leaf.
            if (th and is_prologue(t) is True) or (not th and (is_prologue(t) is False or reaches_return(t, th))):
                new[t] = "pointer"; forced[t] = th
        # prologue scan in uncovered code-area gaps
        for lo, hi in CODE_SPANS:
            x = lo
            while x < hi:
                if free(x) and x not in origin and x not in new:
                    p_ = is_prologue(x)
                    if p_ is not None:
                        th = p_ is True
                        if plausible(x, th, 6) and not shadowed_by_arm(x, th):
                            new[x] = "prologue"; forced[x] = th
                x += 2
        # last resort (only when nothing else was found this round): the first byte of each
        # remaining gap inside a code span, if it decodes as a straight run that returns.
        if not new:
            for lo, hi in CODE_SPANS:
                x = lo
                while x < hi:
                    if free(x) and not IMG[x - BASE] == 0:
                        g = x if (x & 3) == 0 or not ((x + 3) & ~3) < hi else x
                        ga = (x + 3) & ~3
                        if (ga - x) <= 2 and ga != x and free(ga) and plausible(ga, False, 4)                                 and unconditional_head(ga, False, 3) and reaches_return(ga, False, 200):
                            new[ga] = "gap-code"; forced[ga] = False
                        elif (g & 3) == 0 and plausible(g, False, 4) and unconditional_head(g, False, 3) \
                                and reaches_return(g, False, 64):
                            new[g] = "gap-code"; forced[g] = False
                        elif (g & 1) == 0 and plausible(g, True, 6) and unconditional_head(g, True, 4) \
                                and (is_prologue(g) is True or decode(g, True).size == 4) \
                                and not shadowed_by_arm(g, True) and reaches_return(g, True, 400):
                            new[g] = "gap-code"; forced[g] = True
                        while x < hi and free(x):
                            x += 1
                    x += 1
        new = {k: v for k, v in new.items() if k not in origin}
        if not new or rounds > 40:
            break
        origin.update(new)
    for s_, f in funcs.items():
        f.origin = origin.get(s_, "?")

    # ----------------------------------------------------------- byte classification
    order = sorted(funcs)
    lit = {}
    for f in funcs.values():
        for la, sz in f.literals.items():
            lit[la] = max(lit.get(la, 0), sz)
    regions = []
    regions.append({"start": BASE, "end": BASE + 0x20, "kind": "code", "name": "vectors", "mode": "arm",
                    "evidence": "STATIC-CONFIRMED"})
    regions.append({"start": BASE + 0x20, "end": BASE + 0x40, "kind": "data", "name": "vector_addresses",
                    "evidence": "STATIC-CONFIRMED"})
    cursor = BASE + 0x40
    overlaps = []
    for s_ in order:
        f = funcs[s_]
        if s_ < cursor:
            overlaps.append((s_, cursor))
            continue
        if s_ > cursor:
            regions.extend(classify_gap(cursor, s_, lit))
        regions.append({"start": s_, "end": f.end, "kind": "function", "mode": "thumb" if f.thumb else "arm",
                        "evidence": "DERIVED"})
        cursor = f.end
    if cursor < LOAD_END:
        regions.extend(classify_gap(cursor, LOAD_END, lit))
    regions.append({"start": LOAD_END, "end": END, "kind": "padding", "name": "packer_garbage",
                    "evidence": "DERIVED",
                    "note": "past the zero-init base; zeroed at boot; Windows stack residue from the packer"})
    # merge adjacent region slices
    total = sum(r["end"] - r["start"] for r in regions)
    assert total == len(IMG), (hex(total), hex(len(IMG)))
    json.dump({"base": BASE, "size": len(IMG), "load_end": LOAD_END, "rounds": rounds,
               "functions": [funcs[s].to_json() for s in order], "regions": regions,
               "overlaps": overlaps}, open(out_dir / "map.json", "w"), indent=0)
    # quick report
    by = {}
    for r in regions:
        by.setdefault(r["kind"], [0, 0])
        by[r["kind"]][0] += 1
        by[r["kind"]][1] += r["end"] - r["start"]
    print(f"rounds={rounds} functions={len(funcs)} (thumb={sum(f.thumb for f in funcs.values())}) overlaps={len(overlaps)}")
    for k, (n, b) in sorted(by.items()):
        print(f"  {k:10s} {n:6d} regions {b:8d} bytes ({100*b/len(IMG):5.2f}%)")


def classify_gap(a, b, lit):
    """Split a non-code gap into literal pool / padding / data runs."""
    out = []
    def emit(kind, s, e, **kw):
        if s >= e:
            return
        if out and out[-1]["kind"] == kind and out[-1]["end"] == s and out[-1].get("name") == kw.get("name"):
            out[-1]["end"] = e
        else:
            out.append(dict(start=s, end=e, kind=kind, **kw))
    x = a
    while x < b:
        if x in lit:
            e = min(x + lit[x], b)
            emit("pool", x, e, evidence="DERIVED")
            x = e
            continue
        if IMG[x - BASE] == 0:
            e = x
            while e < b and IMG[e - BASE] == 0 and e not in lit:
                e += 1
            # Short zero runs between data bytes are part of the data (bitmaps, tables).
            if (e - x) < 16 and x != a and e != b and out and out[-1]["kind"] == "data":
                emit("data", x, e, evidence="OPEN")
            else:
                emit("padding", x, e, name="zero", evidence="STATIC-CONFIRMED")
            x = e
            continue
        e = x + 1
        while e < b and e not in lit and IMG[e - BASE] != 0:
            e += 1
        if out and out[-1]["kind"] == "padding" and out[-1]["end"] == x and out[-1]["end"] - out[-1]["start"] < 16                 and out[-1]["start"] != a:
            out[-1]["kind"] = "data"; out[-1]["evidence"] = "OPEN"; out[-1].pop("name", None)
            if len(out) > 1 and out[-2]["kind"] == "data" and out[-2]["end"] == out[-1]["start"]:
                out[-2]["end"] = out[-1]["end"]; out.pop()
        emit("data", x, e, evidence="OPEN")
        x = e
    return out


def gap_region(a, b):
    chunk = IMG[a - BASE:b - BASE]
    if not any(chunk):
        return {"start": a, "end": b, "kind": "padding", "name": "zero", "evidence": "STATIC-CONFIRMED"}
    if b - a <= 2 and chunk in (b"\x00\xbf", b"\xc0\x46"):
        return {"start": a, "end": b, "kind": "padding", "name": "thumb_nop_align", "evidence": "STATIC-CONFIRMED"}
    return {"start": a, "end": b, "kind": "data", "evidence": "OPEN"}


if __name__ == "__main__":
    main()
