"""Behavioural tests for batch 11: FPGA start-up, SMEC jobs, touch service, power, disk glue.

The FPGA bus primitives (selector write, data read/write) are stubs driven by a seeded model kept
on the machine, so the stock code and the C see the same answers; every selector and data byte
is part of the compared call sequence. Callees that receive stack buffers record contents, not
addresses.
"""
import random as _random

from unicorn import arm_const as A

from tools.emu.emu import Stub
from tools.emu.tests.fatfs import Case

N = 200
V, R = 0, 32
SYS = 0x40544E68
SMEC, TIMING = 0x42E62C2C, 0x42E67558
TOUCH, TOUCH_LAST, TICK, HOLD = 0x42E68BB0, 0x42E68C70, 0x42E68CD8, 0x42E61FF8
PE_MODE = 0x42E68D19
SD_STATE, SECTOR_SIZE, SD_INFO, BOUNCE = 0x40095490, 0x4009548C, 0x42E68A18, 0x427EB634
BUF = 0x41000000
FS = 0x41100000            # fake FATFS returned by f_getfree


def rnd_words(rng, base, n):
    vals = [rng.getrandbits(32) for _ in range(n)]
    return lambda m: [m.w32(base + 4 * i, v) for i, v in enumerate(vals)]


def compose(*fns):
    return lambda m: [f(m) for f in fns]


def regs3(m):
    return [m.reg(x) for x in (A.UC_ARM_REG_R0, A.UC_ARM_REG_R1, A.UC_ARM_REG_R2, A.UC_ARM_REG_R3)]


def rec(name, addr, nargs, ret=None):
    return Stub(name, addr, (lambda m, a: ret if not callable(ret) else ret(m, a)), nargs)


# ---------------------------------------------------------------- FPGA bus model
class FpgaModel:
    def __init__(self, seed, p_ready=0.5, p_echo=0.6, p_valid=0.7, p_id=0.5):
        self.rng = _random.Random(seed)
        self.sel = 0
        self.job = 0
        self.idn = 0
        self.p = (p_ready, p_echo, p_valid, p_id)
        self.id_ok = self.rng.random() < p_id

    def read(self):
        r, (p_ready, p_echo, p_valid, _) = self.rng, self.p
        s = self.sel
        if s in (0x64, 0x65):
            return (r.getrandbits(8) & ~1) | (1 if r.random() < p_ready else 0)
        if s == 0x68:
            return self.job & 0xFF if r.random() < p_echo else r.getrandbits(8)
        if s == 0x67:
            return (r.getrandbits(8) & ~1) | (1 if r.random() < p_valid else 0)
        if s == 0x01:
            v = (0xA5, 0xA5, 0x5A, 0x5A)[self.idn % 4] if self.id_ok else r.getrandbits(8)
            self.idn += 1
            return v
        return r.getrandbits(8)


def fpga_stubs():
    def cmd(m, a):
        m.fpga.sel = a[0] & 0xFF

    def wr(m, a):
        if m.fpga.sel == 0x6A:
            m.fpga.job = a[0]

    def rd(m, a):
        return m.fpga.read()
    return (Stub("fpga_cmd", 0x4004927C, cmd, 1), Stub("fpga_wr", 0x40049214, wr, 1), Stub("fpga_rd", 0x400491B8, rd, 0))


def with_fpga(seed, inner=None, **kw):
    def setup(m):
        if inner:
            inner(m)
        m.fpga = FpgaModel(seed, **kw)
    return setup


TESTS = {}


def seeded(seed, inner=None):
    """setup that also (re)seeds the stub random stream for this run"""
    def setup(m):
        if inner:
            inner(m)
        m.srng = _random.Random(seed)
    return setup


def smec_state(rng):
    """SMEC cache/valid flags and the timing table, partly matching typical values."""
    cache = [rng.choice([0, 1, 2, rng.getrandbits(32)]) for _ in range(8)]
    valid = [rng.choice([0, 1]) for _ in range(8)]
    timing = [rng.choice([2000, 2900, 3000, rng.randrange(2000, 3001)]) for _ in range(255)]
    pend = rng.choice([0, 1])

    def setup(m):
        for i in range(8):
            m.w32(SMEC + 0x3FC + 4 * i, cache[i]); m.w8(SMEC + 0x7F8 + i, valid[i])
        m.w8(SMEC + 0x8F7, pend)
        for i, t in enumerate(timing):
            m.w32(TIMING + 4 * i, t)
    return setup, cache


def fcase(args_fn, ret=V, state=None, extra=(), **kw):
    def mk(r):
        seed = r.getrandbits(32)
        inner = state(r)[0] if state else None
        p = {k: (v(r) if callable(v) else v) for k, v in kw.items()}
        return Case(args_fn(r), with_fpga(seed, inner, **p), ret=ret, stubs=fpga_stubs() + tuple(extra))
    return mk


def small_or_big(r):
    return r.getrandbits(8) if r.random() < 0.6 else r.getrandbits(32)


# selectors, job steps
TESTS[0x40049F60] = fcase(lambda r: [small_or_big(r)])
TESTS[0x40049F7C] = fcase(lambda r: [r.getrandbits(32)])
TESTS[0x40049FB8] = fcase(lambda r: [r.getrandbits(32) if r.random() < 0.3 else r.getrandbits(16)])
TESTS[0x40049FDC] = fcase(lambda r: [], p_ready=lambda r: r.choice([0.2, 0.5, 0.9]))
TESTS[0x4004A020] = fcase(lambda r: [], ret=R)
TESTS[0x4004A040] = fcase(lambda r: [], p_ready=lambda r: r.choice([0.2, 0.5, 0.9]))
TESTS[0x4004A064] = fcase(lambda r: [], ret=R)
TESTS[0x4004A080] = fcase(lambda r: [], ret=R)
TESTS[0x4004A13C] = fcase(lambda r: [])
TESTS[0x40049F44] = lambda r: Case([], rnd_words(r, TIMING, 260), ret=V)
TESTS[0x4004A0B0] = fcase(lambda r: [r.randrange(8), r.getrandbits(32)], ret=R, state=smec_state,
                          p_echo=lambda r: r.choice([0.0, 0.2, 0.6, 1.0]), p_valid=lambda r: r.choice([0.0, 0.5, 1.0]))
TESTS[0x40049F1C] = fcase(lambda r: [r.randrange(8)], state=smec_state, p_echo=0.8, p_valid=0.8)
TESTS[0x40049E00] = fcase(lambda r: [], state=smec_state, p_echo=0.8, p_valid=0.8)


def updater_args(r):
    idx = r.randrange(8)
    return [idx, r.choice([1, 2, 4, 4, 3]), r.getrandbits(32), r.getrandbits(32)]


def upd_case(r):
    setup, cache = smec_state(r)
    args = updater_args(r)
    if r.random() < 0.4:                     # value equal to the cached one
        args[2] = args[3] = cache[args[0]]
    return Case(args, setup, ret=V, stubs=(rec("smec_run_job", 0x4004A0B0, 2, ret=0),))


TESTS[0x40049EC4] = upd_case


def sys_params(rng):
    vals = {o: rng.getrandbits(32) for o in (0x08, 0x1B8, 0x1C0, 0x268, 0x26C, 0x270, 0x62C)}
    return lambda m: [m.w32(SYS + o, v) for o, v in vals.items()]


TESTS[0x40049E30] = lambda r: Case([], sys_params(r), ret=V, stubs=(rec("fpga_param_updater", 0x40049EC4, 4),))


def smec_init_case(r):
    seed = r.getrandbits(32)
    init = compose(sys_params(r), rnd_words(r, SMEC, 0x8FC // 4), rnd_words(r, TIMING, 255))
    return Case([], with_fpga(seed, init, p_echo=0.9, p_valid=0.9), ret=V, stubs=fpga_stubs())


TESTS[0x4004A15C] = smec_init_case

# start-up
START_STUBS = (rec("csi_full_init", 0x400660C8, 1), rec("fpga_bus_init", 0x40049110, 0), rec("csi_pins_init_0", 0x400660C0, 0),
               rec("ms5351_pins_init", 0x4005F1BC, 0), rec("ms5351_set_clk0", 0x4005F1F0, 1), rec("ms5351_set_clk1", 0x4005F38C, 1))


def mode_case(r):
    cur = r.choice([0, 1, 2, 0xFF])
    arg = r.choice([0, 1, 2, cur, 0x100, r.getrandbits(32)])
    return Case([arg], lambda m: m.w8(PE_MODE, cur), ret=V, stubs=START_STUBS)


TESTS[0x40049094] = mode_case
TESTS[0x400490B8] = lambda r: Case([], ret=V, stubs=START_STUBS)
TESTS[0x40049188] = lambda r: Case([], (lambda c: lambda m: m.w8(PE_MODE, c))(r.choice([0, 1, 5])), ret=V, stubs=START_STUBS)
TESTS[0x400492E4] = fcase(lambda r: [], p_id=lambda r: r.choice([0.0, 0.5, 1.0]))
TESTS[0x4004A278] = fcase(lambda r: [])


# ---------------------------------------------------------------- touch
def gt_stubs():
    def rd(m, a):
        rng = m.srng
        reg, buf, n = regs3(m)[:3]
        if reg == 0x814E:
            st = rng.choice([0x00, 0x80, 0x81, 0x82, 0x05, rng.getrandbits(8)])
            data = bytes([st]) + bytes(rng.getrandbits(8) for _ in range(n - 1))
        else:
            vals = [rng.choice([rng.randrange(0, 1100), rng.randrange(0, 700), rng.getrandbits(16)]) for _ in range(2)]
            data = b"".join(v.to_bytes(2, "little") for v in vals)[:n]
        m.write(buf, data)
        m.calls.append(("gt911_read16", reg, n))

    def wr(m, a):
        reg, buf, n = regs3(m)[:3]
        m.calls.append(("gt911_write16", reg, bytes(m.read(buf, n)), n))
    return (Stub("gt911_read16*", 0x400498D4, rd, 0), Stub("gt911_write16*", 0x400498C4, wr, 0))


def touch_state(rng):
    blk = bytearray(rng.choice([0, 1, 2, rng.getrandbits(8)]) for _ in range(0x18))
    blk[0x14] = rng.choice([0, 0, 1])          # busy: mostly free, so the service runs
    blk[0x16] = rng.choice([0, 1])             # enabled
    blk = bytes(blk)
    tick = rng.getrandbits(32) if rng.random() < 0.3 else rng.randrange(100, 1 << 20)
    last = rng.choice([tick - 20, tick - 15, tick - 16, tick, tick - 1000, rng.getrandbits(32)]) & 0xFFFFFFFF
    hold = rng.choice([0, 1])

    def setup(m):
        m.write(TOUCH, blk); m.w32(TICK, tick); m.w32(TOUCH_LAST, last); m.w8(HOLD, hold)
    return setup


def tcase(extra=()):
    def mk(r):
        seed = r.getrandbits(32)
        return Case([], seeded(seed, touch_state(r)), ret=V, stubs=gt_stubs() + tuple(extra))
    return mk


TESTS[0x400498E4] = tcase()
TESTS[0x40049A9C] = tcase()
TESTS[0x40049B08] = tcase()
TESTS[0x40049A38] = lambda r: Case([], touch_state(r), ret=V)
GT_INIT = (rec("gt911_i2c_pins_init", 0x40049788, 0), rec("gt911_ctrl_pins_out", 0x400497D4, 0), rec("gt911_bus_init", 0x400498A4, 0))
TESTS[0x40049A5C] = lambda r: Case([], ret=V, stubs=GT_INIT)
TESTS[0x40049A74] = lambda r: Case([], touch_state(r), ret=V, stubs=GT_INIT)


# ---------------------------------------------------------------- power
def power_state(rng):
    w = {o: rng.getrandbits(16) for o in (4, 6, 8, 0x0A)}
    minutes = rng.choice([0, 0, 1, 5, rng.getrandbits(32)])
    tick = rng.getrandbits(32)
    deadline = rng.choice([tick - 1, tick, tick + 1, rng.getrandbits(32)]) & 0xFFFFFFFF
    p = 0x42C2FE74 + 0xDB60
    img, dx, dy = rng.getrandbits(32), rng.getrandbits(8), rng.getrandbits(8)

    def setup(m):
        for o, v in w.items():
            m.write(SYS + o, v.to_bytes(2, "little"))
        m.w32(SYS + 0x658, minutes); m.w32(SYS + 0x65C, deadline); m.w32(TICK, tick)
        m.w32(p, img); m.w8(p + 0x194, dx); m.w8(p + 0x195, dy)
    return setup


POWER_STUBS = (rec("func_40034134", 0x40034134, 4), rec("delay_ms", 0x40042CF0, 1), rec("power_off", 0x4004A1B4, 0),
               rec("lradc_init", 0x40068E94, 0))
TESTS[0x4004A1D0] = lambda r: Case([], power_state(r), ret=V)
TESTS[0x4004A1F4] = lambda r: Case([], power_state(r), ret=V, stubs=POWER_STUBS)
TESTS[0x4004A2A0] = lambda r: Case([], rnd_words(r, 0x42E68C94, 3), ret=V)
TESTS[0x4004A198] = lambda r: Case([], compose(power_state(r), rnd_words(r, 0x42E68C94, 3)), ret=V, stubs=POWER_STUBS)


# ---------------------------------------------------------------- disk glue
def disk_state(rng):
    st = rng.choice([1, 1, 1, 0, 2])
    ss = rng.choice([512, 512, 1024, 4096])
    info = rng.getrandbits(32)
    data = bytes(rng.getrandbits(8) for _ in range(4 * 4096))

    def setup(m):
        m.w32(SD_STATE, st); m.w32(SECTOR_SIZE, ss); m.w32(SD_INFO + 4, info)
        m.write(BUF, data); m.write(BOUNCE, data[::-1])
    return setup


def mmc_stubs():
    def rd(m, a):
        rng = m.srng
        n = min(a[2], 4) * 512
        m.write(a[3], bytes(rng.getrandbits(8) for _ in range(n)))
        return rng.choice([0, 0, 1, 5])

    def wr(m, a):
        rng = m.srng
        m.calls.append(("mmc_write_data", bytes(m.read(a[3], min(a[2], 4) * 512))))
        return rng.choice([0, 0, 1, 2])
    return (Stub("mmc_read_blocks", 0x4006866C, rd, 4), Stub("mmc_write_blocks", 0x40068794, wr, 4),
            Stub("mmc_card_init", 0x40068918, lambda m, a: m.srng.choice([0, 0, 1, 3]), 1))


def dcase(args_fn, ret=R):
    def mk(r):
        seed = r.getrandbits(32)
        return Case(args_fn(r), seeded(seed, disk_state(r)), ret=ret, stubs=mmc_stubs())
    return mk


def pdrv(r):
    return r.choice([0, 0, 0, 1, 2])


TESTS[0x40049C38] = dcase(lambda r: [pdrv(r)])
TESTS[0x40049C78] = dcase(lambda r: [pdrv(r), BUF + 0x8000, r.getrandbits(32), r.randrange(1, 5)])
TESTS[0x40049CE8] = dcase(lambda r: [pdrv(r), BUF, r.getrandbits(32), r.randrange(1, 5)])
TESTS[0x40049D68] = dcase(lambda r: [pdrv(r), r.choice([0, 1, 2, 3, 4, 9]), BUF + 0x9000])
TESTS[0x40049DD8] = lambda r: Case([], ret=R)


def ff_stubs():
    def getfree(m, a):
        rng = m.srng
        path, ncl, fsp = regs3(m)[:3]
        m.calls.append(("f_getfree", path))
        res = rng.choice([0, 0, 0, 1, 3])
        if res == 0:
            m.w32(ncl, rng.getrandbits(32) if rng.random() < 0.2 else rng.randrange(0, 1 << 20))
            m.w32(fsp, FS)
            m.write(FS + 0x0A, rng.choice([1, 8, 64, rng.getrandbits(16)]).to_bytes(2, "little"))
        return res

    def mkfs(m, a):
        rng = m.srng
        sp = m.reg(A.UC_ARM_REG_SP)
        q = regs3(m)
        m.calls.append(("f_mkfs", q[0], q[1], q[2], q[3], m.r32(sp)))
        return rng.choice([0, 1, 14])
    return (Stub("f_mount", 0x40059AEC, lambda m, a: m.srng.choice([0, 0, 1, 3, 13]), 3),
            Stub("f_getfree*", 0x4005ABC0, getfree, 0), Stub("f_mkfs*", 0x4005B1F8, mkfs, 0))


def ffcase(ret=R):
    def mk(r):
        seed = r.getrandbits(32)
        return Case([], seeded(seed, rnd_words(r, FS, 4)), ret=ret, stubs=ff_stubs())
    return mk


TESTS[0x40049B80] = ffcase()
TESTS[0x40049B90] = ffcase()
TESTS[0x40049BB4] = ffcase()
TESTS[0x40049BE4] = ffcase()
