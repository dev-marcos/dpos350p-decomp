"""Records the serializer calls of a stock field-list function and prints them as C, with runs that
repeat with a constant pointer stride folded into loops.

Usage: gen_call_list.py ENTRY_HEX [ADDR=VALUE ...]   (bytes to set before the run, e.g. counts = 0)"""
import sys
sys.path.insert(0, '.')
from tools.emu.emu import Machine, Stub

entry = int(sys.argv[1], 16)
pre = [tuple(int(x, 16) for x in a.split('=')) for a in sys.argv[2:]]
calls = []
st = [Stub("cur", 0x40067128, lambda m, a: calls.append(("cur", a[0], 0)), 1),
      Stub("fld", 0x40067450, lambda m, a: calls.append(("fld", a[0], a[1])), 2)]
m = Machine(stubs=st, max_insns=50_000_000)
for a, v in pre:
    m.w8(a, v)
res = m.call(entry, [])
assert not res["error"], res["error"]
BASES = [("SYS", 0x40544E68), ("BLK", 0x42E68668), ("BODE", 0x42E180C8), ("0x42E22D28u", 0x42E22D28), ("0x42E3F984u", 0x42E3F984), ("0x42E53A0Cu", 0x42E53A0C), ("0x42E3C000u", 0x42E3C000)]


def ptr(a):
    best = None
    for n, b in BASES:
        if 0 <= a - b < 0x20000 and (best is None or a - b < best[1]):
            best = (n, a - b)
    return "%s + 0x%X" % best if best else "0x%08Xu" % a


out = []
i = 0
N = len(calls)
while i < N:
    best = (1, 1, 0)                   # (covered, period, stride)
    for p in range(1, 41):
        if i + 2 * p > N:
            break
        d = calls[i + p][1] - calls[i][1]
        k = 1
        while i + (k + 1) * p <= N and all(
                calls[i + k * p + t][0] == calls[i + t][0] == "fld" and
                calls[i + k * p + t][2] == calls[i + t][2] and
                calls[i + k * p + t][1] == calls[i + t][1] + k * d for t in range(p)):
            k += 1
        if k >= 3 and k * p > best[0]:
            best = (k * p, p, d)
    cov, p, d = best
    if cov == 1:
        k, a, s = calls[i]
        if k == "cur":
            out.append("    serializer_set_cursor(%d);" % a)
        else:
            out.append("    FLD(%s, %d);" % (ptr(a), s))
        i += 1
        continue
    k = cov // p
    out.append("    for (j = 0; j < %d; j++) {" % k)
    for t in range(p):
        out.append("        FLD(%s %s 0x%X * j, %d);" % (ptr(calls[i + t][1]), "-" if d < 0 else "+", abs(d), calls[i + t][2]))
    out.append("    }")
    i += cov
print("\n".join(out))
print(N, "calls,", len(out), "lines", file=sys.stderr)
