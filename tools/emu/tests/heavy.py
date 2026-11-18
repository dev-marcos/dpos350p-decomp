"""Behavioural tests for functions that take seconds in the emulator (whole-screen loops), with
few cases per function. Kept apart so that their cost does not slow down sweep.py."""
from tools.emu.emu import Stub
from tools.emu.tests.fatfs import Case

N = 6
SYS = 0x40544E68
TESTS = {}


def rec(name, addr, nargs, ret=None):
    return Stub(name, addr, (lambda m, a: ret if not callable(ret) else ret(m, a)), nargs)


# ================================================================ batch 48: screenshot preview
def t_sthumb48(r):
    kind = r.randrange(4)
    if kind == 0:
        pix = bytes(r.getrandbits(8) for _ in range(2 * 1022 * 600))
    else:
        c = [0xFFFF, 0x0000, r.getrandbits(16)][kind - 1]
        pix = c.to_bytes(2, "little") * (1022 * 600)
    hdr = r.getrandbits(32)

    def setup(m):
        m.max_insns = 200_000_000
        m.write(0x42210D20, pix)
        m.w32(SYS + 0x109684, hdr)
    return Case([], setup=setup, ret=0, stubs=(rec("copy", 0x40035728, 3),))


TESTS[0x40034E94] = t_sthumb48
