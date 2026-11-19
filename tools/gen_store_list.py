"""Turns a straight-line store-only stock function into C statements, in execution order.

Runs the stock code twice with different random memory; each store whose value is the same in both
runs is a constant, otherwise it must equal (the low bits of) a value read earlier in both runs,
and becomes a copy of that address. Usage: gen_store_list.py ENTRY_HEX"""
import random
import sys
sys.path.insert(0, '.')
from unicorn import UC_HOOK_MEM_WRITE, UC_HOOK_MEM_READ
from tools.emu.emu import Machine

entry = int(sys.argv[1], 16)
REGIONS = [(0x42E64520, 0x1000), (0x40544E68, 0x40), (0x42E68668, 0x200), (0x42E61FF8, 0x1000), (0x42E18000, 0x200)]


def run(seed):
    m = Machine()
    r = random.Random(seed)
    for a, n in REGIONS:
        m.write(a, bytes(r.getrandbits(8) for _ in range(n)))
    ev = []

    def wh(uc, acc, addr, size, value, u):
        if 0x40000000 <= addr < 0x44000000 and not (0x43F00000 <= addr):
            ev.append(("W", addr, size, value & ((1 << (8 * size)) - 1)))

    def rh(uc, acc, addr, size, value, u):
        if 0x40100000 <= addr < 0x43F00000:
            ev.append(("R", addr, size, int.from_bytes(uc.mem_read(addr, size), "little")))
    m.uc.hook_add(UC_HOOK_MEM_WRITE, wh)
    m.uc.hook_add(UC_HOOK_MEM_READ, rh)
    res = m.call(entry, [])
    assert not res["error"], res["error"]
    return ev


e1, e2 = run(1), run(2)
assert [x[:3] for x in e1] == [x[:3] for x in e2]
M = {1: "B8", 2: "H16", 4: "W32"}
CAST = {1: "(unsigned char)", 2: "(unsigned short)", 4: ""}
out = []
reads = []
for (k, a, s, v1), (_, _, _, v2) in zip(e1, e2):
    if k == "R":
        reads.append((a, s, v1, v2))
        continue
    mask = (1 << (8 * s)) - 1
    if v1 == v2:
        out.append("    %s(0x%08Xu) = 0x%X;" % (M[s], a, v1))
        continue
    src = [x for x in reversed(reads) if x[2] & mask == v1 and x[3] & mask == v2]
    assert src, hex(a)
    ra, rs = src[0][0], src[0][1]
    out.append("    %s(0x%08Xu) = %s%s(0x%08Xu);" % (M[s], a, CAST[s] if rs > s else "", M[rs], ra))
print("\n".join(out))
print(len(out), "stores", file=sys.stderr)
written = set()
for k, a, s, v in e1:
    if k == "W":
        written.update(range(a, a + s))
    else:
        inreg = any(lo <= a < lo + n for lo, n in REGIONS)
        print("read %08X size %d %s" % (a, s, "written-before" if a in written else ("random" if inreg else "UNCOVERED")), file=sys.stderr)
