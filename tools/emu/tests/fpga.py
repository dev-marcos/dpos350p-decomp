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


# ================================================================ batch 12: battery, front end, POST
import struct as _struct


def f32(x):
    return _struct.unpack("<I", _struct.pack("<f", x))[0]


def sys_bytes(rng, lo=0, hi=0xC00, overrides=None):
    """random SYS_INFO bytes in [lo, hi) plus fixed overrides {offset: (size, value)}"""
    data = bytearray(rng.getrandbits(8) for _ in range(hi - lo))
    ov = overrides or {}

    def setup(m):
        m.write(SYS + lo, bytes(data))
        for o, (sz, v) in ov.items():
            m.write(SYS + o, (v & ((1 << (8 * sz)) - 1)).to_bytes(sz, "little"))
    return setup


def batt_coeffs(rng):
    if rng.random() < 0.7:
        return f32(32.947), f32(2565.74), f32(170.0)
    return f32(rng.uniform(-50, 80)), f32(rng.uniform(-5000, 5000)), f32(rng.uniform(-500, 500))


BATT_THRESH = [4150, 4080, 3970, 3900, 3840, 3790, 3760, 3730, 3690, 3620, 3560]


def t_batt_level(r):
    code = r.choice([r.randrange(15, 50), r.randrange(0, 64), r.getrandbits(32) - (1 << 31)])
    sl, ba, of = batt_coeffs(r)
    if r.random() < 0.5:           # land exactly on / next to a level threshold
        code = r.randrange(0, 64)
        sl, ba, of = f32(1.0), f32(r.choice(BATT_THRESH) + r.choice([-1, 0, 1]) - code), f32(0.0)
    ov = r.getrandbits(8)

    def setup(m):
        m.w32(0x42E68CA4, code & 0xFFFFFFFF); m.w32(0x42E68C94, sl); m.w32(0x42E68C98, ba); m.w32(0x42E68C9C, of)
        m.w8(SYS + 0x778, ov)
    return Case([], setup, ret=V)


TESTS[0x4004A2D0] = t_batt_level

BATT_STUBS = (rec("timer_stop", 0x40042C40, 1), rec("func_400342cc", 0x400342CC, 0), rec("func_40034134", 0x40034134, 4),
              rec("delay_ms", 0x40042CF0, 1), rec("power_off", 0x4004A1B4, 0))


def t_batt_empty(r):
    p = 0x42C2FE74 + 0xDD00
    img, dx, dy = r.getrandbits(32), r.getrandbits(8), r.getrandbits(8)
    lvl = r.choice([0, 0, 1, 5, r.getrandbits(8)])
    base = sys_bytes(r, 0, 0x20)

    def setup(m):
        base(m); m.w8(SYS + 0x778, lvl); m.w32(p, img); m.w8(p + 0x194, dx); m.w8(p + 0x195, dy)
    return Case([], setup, ret=V, stubs=BATT_STUBS)


TESTS[0x4004A3C0] = t_batt_empty


def t_batt_service(r):
    tick = r.getrandbits(32)
    last = r.choice([tick - 1000, tick - 1001, tick - 5000, tick, r.getrandbits(32)]) & 0xFFFFFFFF
    cnt = r.choice([0, 2, 3, 10, r.getrandbits(32)])
    s = r.getrandbits(32)
    avg = r.getrandbits(32)

    def setup(m):
        m.w32(TICK, tick); m.w32(0x42E68CAC, last); m.w32(0x42E68CA8, cnt); m.w32(0x42E68CA0, s); m.w32(0x42E68CA4, avg)
    stubs = (rec("charge_status_read", 0x4004A278, 0), rec("lradc_read", 0x40068E74, 0, ret=lambda m, a: m.srng.getrandbits(8)),
             rec("battery_level_update", 0x4004A2D0, 0), rec("battery_empty_check", 0x4004A3C0, 0), rec("auto_off_check", 0x4004A1F4, 0))
    return Case([], seeded(r.getrandbits(32), setup), ret=V, stubs=stubs)


TESTS[0x4004A448] = t_batt_service
TESTS[0x4004A550] = lambda r: Case([], sys_bytes(r, 0xEE0, 0xF60), ret=V)
TESTS[0x4004A50C] = lambda r: Case([], sys_bytes(r, 0xEE0, 0xF60), ret=V)

FE_CALLEES = [("offset_clamp", 0x4004A7C4), ("vscale_update", 0x4004A608), ("cal_selector", 0x4004C544), ("vdiv_labels_update", 0x4004AA04),
              ("sel62", 0x4004AA70), ("ch1_frontend_apply", 0x4004AAC0), ("ch2_frontend_apply", 0x4004AC04), ("sel2f", 0x4004AB60),
              ("sel29", 0x4004AB84), ("sel30", 0x4004ACA4), ("sel2c", 0x4004ABA8), ("channel_mode", 0x4004AA24)]
TESTS[0x4004A510] = lambda r: Case([], ret=V, stubs=tuple(rec(n, a, 0) for n, a in FE_CALLEES))


def t_vscale(r):
    ov = {0x40: (1, r.choice([0, 1, 2, 3, 7])), 0x104: (1, r.choice([0, 1, 2, 3, 9])),
          0x41: (1, r.choice([r.randrange(13), r.getrandbits(8)])), 0x105: (1, r.choice([r.randrange(13), r.getrandbits(8)])),
          0x1B9: (1, r.choice([r.randrange(14), r.getrandbits(8)]))}
    return Case([], sys_bytes(r, 0, 0x1800, ov), ret=V)


TESTS[0x4004A608] = t_vscale


def t_offset(r):
    mg = r.choice([r.randrange(0, 60), r.getrandbits(16)])
    hgt = r.choice([503, 503, r.getrandbits(16)])

    def pos():
        return r.choice([r.randrange(-50, 600), mg, hgt - mg, 2 * mg, hgt - 2 * mg, r.getrandbits(32)])
    ov = {0x1AE: (2, mg), 0x2AA: (1, r.choice([0, 1, 5])), 0x0A: (2, hgt), 0x60: (4, pos()), 0x120: (4, pos())}
    return Case([], sys_bytes(r, 0, 0x300, ov), ret=V)


TESTS[0x4004A7C4] = t_offset


def label_value(r):
    return r.choice([r.choice([20000000, 10000000, 5000000, 2000000, 1000000, 500000, 200000, 100000, 50000, 20000, 10000, 5000, 2000])
                     * r.choice([1, 10, 100]), r.randrange(0, 1000), 999, 1000, 999999, 1000000, 999999999, 1000000000,
                     r.getrandbits(64), r.getrandbits(40)])


def t_vdiv_label(r):
    ov = {0x50: (8, label_value(r)), 0x110: (8, label_value(r))}
    d = r.choice([r.randrange(0, 1000), r.getrandbits(32) - (1 << 31), 0, 5, 50, 500])
    unit = r.choice([0, 1, 2, 3, r.getrandbits(32)])
    return Case([r.choice([0, 1, 2]), r.getrandbits(32), d & 0xFFFFFFFF, unit & 0xFFFFFFFF], sys_bytes(r, 0, 0x140, ov), ret=V)


TESTS[0x4004A884] = t_vdiv_label


def t_vdiv_labels(r):
    ov = {0x50: (8, label_value(r)), 0x110: (8, label_value(r))}
    return Case([r.getrandbits(32), r.getrandbits(32), r.randrange(0, 1000), r.choice([0, 1, 2, 3])], sys_bytes(r, 0, 0x140, ov), ret=V)


TESTS[0x4004AA04] = t_vdiv_labels


def sel_case(extra=(), ov_fn=None):
    def mk(r):
        seed = r.getrandbits(32)
        ov = ov_fn(r) if ov_fn else {}
        return Case([], with_fpga(seed, sys_bytes(r, 0, 0xC00, ov)), ret=V, stubs=fpga_stubs() + tuple(extra))
    return mk


DELAY = rec("delay_ms", 0x40042CF0, 1)
TESTS[0x4004AA24] = sel_case((DELAY,), lambda r: {0xBCA: (1, r.choice([0, 1])), 0x100: (1, r.choice([0, 1, 2]))})
for _a in (0x4004AA70, 0x4004AA94, 0x4004AB60, 0x4004AB84, 0x4004ABA8, 0x4004ABCC, 0x4004ACA4):
    TESTS[_a] = sel_case()
FE_STUBS = (DELAY, rec("fpga_param_updater", 0x40049EC4, 4), rec("smec_commit", 0x40049F1C, 1))
TESTS[0x4004AAC0] = sel_case(FE_STUBS, lambda r: {0xBC8: (1, r.choice([0, 6, 12])), 0x41: (1, r.choice([0, 6, 12, 11]))})
TESTS[0x4004AC04] = sel_case(FE_STUBS, lambda r: {0xBC9: (1, r.choice([0, 6, 12])), 0x105: (1, r.choice([0, 6, 12, 11]))})


def lane_stubs():
    def copy(m, a):
        n, buf = a[1], a[2]
        m.write(buf, bytes(m.srng.getrandbits(8) for _ in range(min(n, 256))))
    return (rec("fpga_sample_count_write", 0x4003F948, 1), rec("acquisition_prepare", 0x4003FA10, 0), rec("func_4003fcc0", 0x4003FCC0, 0),
            Stub("lane_copy", 0x400406A4, copy, 3))


def lane_ov(r):
    return {0x41: (1, r.randrange(13)), 0x105: (1, r.randrange(13)), 0x1B8: (1, r.choice([20, 0, 255, r.getrandbits(8)])),
            0x26C: (4, r.choice([r.randrange(1, 100), r.getrandbits(32)]))}


def t_lane(kind):
    def mk(r):
        seed = r.getrandbits(32)
        n = r.choice([0, 1, 8, 64, 200])
        setup = seeded(seed, sys_bytes(r, 0, 0x700, lane_ov(r)))
        if kind == "channel":          # channel_level_mean(start, ch, n, buf)
            args = [r.choice([0, 1]), r.choice([0, 1, 2]), max(n, 1), BUF]
            return Case(args, setup, ret=R, stubs=lane_stubs())
        args = [r.choice([0, 1]), r.choice([r.randrange(8), r.getrandbits(16)]), r.choice([0, 1, 2, r.getrandbits(8)]),
                max(n, 1) if kind == "mean" else n]
        return Case(args, setup, ret=R, stubs=lane_stubs(), stack=[BUF])
    return mk


TESTS[0x4004ACC8] = t_lane("sum")
TESTS[0x4004ADCC] = t_lane("mean")
TESTS[0x4004ADF0] = t_lane("channel")


def t_post_mux(r):
    def dispatch(m, a):
        for pos, meas in ((0x60, 0xB34), (0x120, 0xB50)):
            p = m.r32(SYS + pos)
            m.w32(SYS + meas, (p + m.srng.choice([-7, -6, -4, -3, -2, 0, 2, 3, 4, 6, 7, 1000])) & 0xFFFFFFFF)
    stubs = (rec("sysconfig_exec", 0x4003E708, 0), DELAY, Stub("acquisition_dispatcher", 0x400419CC, dispatch, 0))
    return Case([r.choice([0, 6, 11, 12, r.getrandbits(8)])], seeded(r.getrandbits(32), sys_bytes(r, 0, 0xC00)), ret=R, stubs=stubs)


TESTS[0x4004AF0C] = t_post_mux


# ================================================================ batch 13: generator, calibration selector
GEN = 0x42E3F984
GSET = GEN + 0x2EFC          # settings block (0x42E42880)
CAL = 0x42E53A0C


def w16(m, a, v):
    m.write(a, (v & 0xFFFF).to_bytes(2, "little"))


def t_selector(r):
    obj = bytes(r.getrandbits(8) for _ in range(0x1A90))
    ov = {0x41: (1, r.randrange(20)), 0x105: (1, r.randrange(20)), 0x100: (1, r.choice([0, 1, 2])),
          0x6EC: (1, r.randrange(0, 12)), 0x588: (1, r.randrange(12, 40)), 0x6ED: (1, r.randrange(12, 40))}
    base = sys_bytes(r, 0, 0x700, ov)

    def setup(m):
        base(m); m.write(CAL, obj)
    return Case([], setup, ret=V)


TESTS[0x4004C544] = t_selector


def t_syscal(r):
    obj = bytes(r.getrandbits(8) for _ in range(0x1A90))
    ov = {0x1B9: (1, r.choice([0, 1, 5, 10])), 0x1BA: (1, r.choice([0, 3, 10, 20]))}
    base = sys_bytes(r, 0x100, 0x200, ov)
    tch = bytes(r.getrandbits(8) for _ in range(0x18))

    def setup(m):
        base(m); m.write(CAL, obj); m.write(TOUCH, tch)
    stubs = (rec("func_4003f0c0", 0x4003F0C0, 0), rec("func_4003f0e0", 0x4003F0E0, 0), rec("calib_row", 0x4004BFC8, 1),
             rec("calib_step", 0x4004B0A0, 0), rec("calibration_save_wrapper", 0x40044680, 0),
             rec("post_channel_mux_check", 0x4004AF0C, 1, ret=lambda m, a: m.srng.choice([1, 1, 1, 1, 1, 1, 1, 1, 0, -1, -2])))
    return Case([], seeded(r.getrandbits(32), setup), ret=R, stubs=stubs)


TESTS[0x4004C6B4] = t_syscal


def t_pct(r):
    def num():
        return r.choice([r.randrange(-100, 1000), 0, r.getrandbits(32), r.randrange(1, 50)]) & 0xFFFFFFFF
    return Case([num(), num(), num()], ret=R)


TESTS[0x4004C794] = t_pct


def gen_block(r):
    """settings block G+0x2EFC..+0x80 with plausible values and limits"""
    def i32(v):
        return (v & 0xFFFFFFFF).to_bytes(4, "little")
    words = [r.getrandbits(32) for _ in range(0x80 // 4)]
    words[0] = r.choice([0, 1000, 999999, 1000000, 3000000, 5000001, 10000000, 50000000, 60000000, -5, r.getrandbits(32)])
    words[1] = r.choice([0, 1000, 5000, -10, r.getrandbits(32), r.randrange(0, 20000)])
    words[2] = r.choice([0, 1000, -3000, 9000, -9000, r.getrandbits(32)])
    words[3] = r.choice([0, -1, 500, 1000, 1200, r.getrandbits(32)])
    words[5] = r.choice([14, 13, 5, 20])
    words[6] = r.choice([60000000, 20000000, 1000000, r.getrandbits(32)])
    words[7] = r.choice([10000, 5000, 20000])
    words[8] = r.choice([5000, 10000])
    words[9] = r.choice([999, 1000])
    words[10] = r.choice([10000, 5000])
    words[11] = r.choice([-10000, -5000])
    return b"".join(i32(w) for w in words)


def t_gen_limits(r):
    blk = gen_block(r)
    wave = r.choice(list(range(16)) + [20, 255])
    tch = bytearray(r.getrandbits(8) for _ in range(0x18))
    tch[0x17] = r.choice([0, 0, 0, 1])

    def setup(m):
        m.write(GSET, blk); m.w8(GEN + 0x2EFA, wave); m.write(TOUCH, bytes(tch))
    return Case([r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, 0xFFFFFFFF, r.getrandbits(32)])], setup, ret=V)


TESTS[0x4004CF34] = t_gen_limits


def gen_tables(r, desc_off=0xFAE):
    buf = bytes(r.getrandbits(8) for _ in range(4000))
    buf2 = bytes(r.getrandbits(8) for _ in range(4000))
    src = bytes(r.getrandbits(8) for _ in range(4000))
    length = r.choice([0, 1, 100, 1000, 2000, r.randrange(1, 2000)])
    cnt = r.choice([1, 2, 4, 8, r.randrange(1, 10)])
    top = r.getrandbits(16)
    tot = r.choice([0, 100, 1000, 2000, r.randrange(1, 2000)])
    tch = bytearray(r.getrandbits(8) for _ in range(0x18))
    tch[0x17] = r.choice([0, 0, 0, 1])
    extra = {0xFB0: r.getrandbits(16), 0x2EF6: r.choice([10, 100, 500, r.randrange(1, 1000)]),
             0x2F30: r.choice([1000, 2000, r.getrandbits(16)]), 0x2F48: r.getrandbits(16), 0x2F58: r.choice([1000, 2000, r.getrandbits(16)])}

    def setup(m):
        m.write(GEN + 0xE, buf); m.write(GEN + 0xFB6, buf2); m.write(GEN + 0x1F56, src)
        w16(m, GEN + desc_off, length); w16(m, GEN + desc_off + 2, top); w16(m, GEN + desc_off + 6, cnt)
        w16(m, GEN + 0x2EF8, tot); m.write(TOUCH, bytes(tch))
        for o, v in extra.items():
            m.w32(GEN + o, v) if o in (0x2F30, 0x2F48, 0x2F58) else w16(m, GEN + o, v)
    return setup


TESTS[0x4004D09C] = lambda r: Case([0xFAE, r.getrandbits(32), TOUCH], gen_tables(r), ret=V)
TESTS[0x4004D094] = lambda r: Case([r.getrandbits(32), r.getrandbits(32)], gen_tables(r), ret=V)
TESTS[0x4004D350] = lambda r: Case([r.choice([0, 1, 2, 3, 7, 50, -3]) & 0xFFFFFFFF], gen_tables(r), ret=V)
TESTS[0x4004D5CC] = lambda r: Case([r.choice([0, 1, 2, 3, 7, 50, -3]) & 0xFFFFFFFF], gen_tables(r), ret=V)
TESTS[0x4004D390] = lambda r: Case([], gen_tables(r), ret=V)
TESTS[0x4004D618] = lambda r: Case([], gen_tables(r), ret=V)


def t_load_samples(r):
    cnt = r.choice([0, 10, 1000, 2000, r.randrange(0, 2000)])
    data = bytes(r.getrandbits(8) for _ in range(4000))

    def setup(m):
        m.w32(SYS + 0x3DA53C, (r.getrandbits(16) << 16) | cnt if False else cnt); m.write(SYS + 0x3DA548, data)
    return Case([], setup, ret=V)


TESTS[0x4004D12C] = t_load_samples
TESTS[0x4004D164] = lambda r: Case([], (lambda v: lambda m: w16(m, 0x4091F396, v))(r.getrandbits(16)), ret=V,
                                   stubs=(rec("gen_file_a", 0x40036B00, 0), rec("gen_file_b", 0x40036BDC, 0), rec("gen_apply", 0x40036EA4, 1),
                                          rec("gen_load_samples", 0x4004D12C, 0), rec("gen_update", 0x4004D18C, 0)))
TESTS[0x4004D18C] = lambda r: Case([r.getrandbits(32) for _ in range(4)], ret=V,
                                   stubs=(rec("eeec", 0x4004EEEC, 0), rec("gen_limits", 0x4004CF34, 4), rec("gen_send_params", 0x4004D1B4, 0),
                                          rec("d880", 0x4004D880, 0), rec("ed18", 0x4004ED18, 0), rec("gen_table_repeat_default", 0x4004D094, 0)))


def t_send_params(r):
    blk = bytearray(gen_block(r))
    v = _struct
    v.pack_into("<f", blk, 0x50, r.choice([1.0, 2.5, 0.37, 1000.0]))
    v.pack_into("<I", blk, 0x54, r.choice([1, 10, 100, 1000]))
    v.pack_into("<I", blk, 0x5C, r.choice([1, 3, 64, 1000]))
    for o in (0x68, 0x6C, 0x70):
        v.pack_into("<f", blk, o, r.choice([0.0, 1.5, -3.25, 12345.6, 1e10]))
    v.pack_into("<I", blk, 0x60, r.getrandbits(32))
    v.pack_into("<I", blk, 0x38, r.choice([100, 2000, 65535, r.getrandbits(32)]))
    v.pack_into("<i", blk, 0x18, r.choice([60000000, 20000000, 12000000]))
    wave = r.choice([0, 0, 1, 5])
    tch = bytearray(r.getrandbits(8) for _ in range(0x18))
    tch[0x17] = r.choice([0, 0, 0, 1])

    def setup(m):
        m.write(GSET, bytes(blk)); m.w8(GEN + 0x2EFA, wave); m.write(TOUCH, bytes(tch))
    return Case([], setup, ret=V, stubs=(rec("fpga_param_updater", 0x40049EC4, 4), rec("smec_commit", 0x40049F1C, 1)))


TESTS[0x4004D1B4] = t_send_params


def nn_stub():
    def fn(m, a):
        sp = m.reg(A.UC_ARM_REG_SP)
        q = regs3(m)
        m.calls.append(("nn_resample", q[0], q[1], q[2], q[3], m.r32(sp), m.r32(sp + 4)))
        m.write(0x416431E8, bytes(m.srng.getrandbits(8) for _ in range(0x4E20 + 4000)))
    return Stub("nn_resample*", 0x4005C678, fn, 0)


def t_resample(r):
    sysw = r.choice([1, 100, 250, r.getrandbits(32)])
    tables = gen_tables(r)

    def setup(m):
        tables(m); m.w32(SYS + 0x554, sysw)
    return Case([], seeded(r.getrandbits(32), setup), ret=V, stubs=(nn_stub(),))


TESTS[0x4004D49C] = t_resample
TESTS[0x4004D730] = t_resample



# ================================================================ batch 14: acquisition control
MEMCPY = rec("memcpy", 0x40035724, 3)
for _a in (0x4003F024, 0x4003F034, 0x4003F044, 0x4003F054, 0x4003F088, 0x4003F098, 0x4003F0A8):
    TESTS[_a] = lambda r: Case([], ret=V, stubs=(MEMCPY,))
TESTS[0x4003F064] = lambda r: Case([], ret=V, stubs=(MEMCPY, rec("ef1c", 0x4003EF1C, 0)))
TESTS[0x4003F0C0] = lambda r: Case([], ret=V, stubs=tuple(rec("s%x" % a, a, 0) for a in (0x4003F024, 0x4003F034, 0x4003F044, 0x4003F054)))
TESTS[0x4003F0E0] = lambda r: Case([], ret=V, stubs=tuple(rec("s%x" % a, a, 0) for a in (0x4003F064, 0x4003F088, 0x4003F098, 0x4003F0A8)))
TESTS[0x4003F894] = lambda r: Case([], ret=V, stubs=tuple(rec("s%x" % a, a, 0) for a in (0x4003F2F4, 0x4003F160, 0x40063F04, 0x40063CD0, 0x40063CB0)))
for _a, _t in ((0x4003FAB0, 0x400636BC), (0x4003FAB4, 0x40063714), (0x4003FAE4, 0x40063798)):
    TESTS[_a] = (lambda t: lambda r: Case([r.getrandbits(32)], ret=V, stubs=(rec("t", t, 1),)))(_t)
PIO = (rec("pio_set_bit", 0x40063274, 2), rec("pio_clr_bit", 0x40063288, 2))
TESTS[0x4003F9F8] = lambda r: Case([], ret=V, stubs=PIO)
TESTS[0x4003FA04] = lambda r: Case([], ret=V, stubs=PIO)

# FPGA selector writers/readers
for _a in (0x4003F8B8, 0x4003F8E0, 0x4003F910, 0x4003F9D0, 0x4003FA10):
    TESTS[_a] = sel_case()
TESTS[0x4003F948] = fcase(lambda r: [r.choice([0, 1, 2, 3, 0x80000001, r.getrandbits(32), r.getrandbits(16)])])
TESTS[0x4003F988] = fcase(lambda r: [small_or_big(r)])
TESTS[0x4003FA94] = fcase(lambda r: [small_or_big(r)])
TRIG = rec("fpga_trigger_pos_write", 0x4003F988, 1)
TESTS[0x4003FAB8] = lambda r: Case([], sys_bytes(r, 0x500, 0x700, {0x520: (4, r.choice([1, 7, 1000, 4750, r.getrandbits(32) | 1]))}),
                                   ret=V, stubs=(TRIG,))


def trig_ov(r, loop_safe=False):
    """SYS fields used by the trigger offset arithmetic; non-zero divisors"""
    neg = r.random() < 0.2
    num = r.randrange(0, 10 ** 6) if loop_safe else r.getrandbits(48)
    den = r.randrange(1, 100)
    if neg:
        num, den = -num, -den if r.random() < 0.5 else den
    ov = {0x570: (8, num), 0x590: (8, den), 0x564: (4, r.randrange(1, 10)), 0x560: (4, r.randrange(1, 10)),
          0x558: (4, r.randrange(1, 10)), 0x1B1: (1, r.randrange(1, 5)), 0x2B4: (4, r.getrandbits(32)),
          0x2BC: (4, r.getrandbits(32) if r.random() < 0.7 else r.randrange(0, 100)),
          0x290: (4, r.getrandbits(32) if r.random() < 0.7 else r.randrange(0, 100))}
    if loop_safe:
        ov[0x520] = (4, r.randrange(10 ** 6, 2 ** 32) if r.random() < 0.7 else r.randrange(10 ** 8, 2 ** 32))
        ov[0x274] = (1, r.choice([0, 1, 1, 1, 7]))
        ov[0x275] = (1, r.choice([0, 1, 1, 1, 9]))
    else:
        ov[0x520] = (4, r.choice([1, 3, 1000, 4750, r.getrandbits(32) | 1]))
    return ov


TESTS[0x4003FAE8] = lambda r: Case([], sys_bytes(r, 0x100, 0x700, trig_ov(r)), ret=V,
                                   stubs=(rec("v6bc", 0x4003FAB0, 0), rec("v714", 0x4003FAB4, 0)))
def t_delay(r):
    ov = trig_ov(r, True)
    if r.random() < 0.3:          # delay exactly 0 or -L before the wrap loop (the d < 1 boundary)
        L = r.randrange(1, 10 ** 6)
        h = r.randrange(L, 10 ** 7)
        ov.update({0x570: (8, 0), 0x520: (4, L), 0x2BC: (4, 2 * h), 0x290: (4, h - r.choice([0, L, 2 * L]) + r.choice([0, 0, 1])),
                   0x274: (1, 1), 0x275: (1, 1)})
    return Case([], sys_bytes(r, 0x100, 0x700, ov), ret=V, stubs=(rec("trigger_pos_update", 0x4003FAB8, 0),))


TESTS[0x4003FB90] = t_delay


def t_wait(r):
    done_at = r.choice([1, 2, 5, 2499, 2500, 2501, 2503, 2600])
    touch_at = r.choice([1, 2, 3, 50, 10 ** 6])
    tch = bytes(r.getrandbits(8) for _ in range(0x18))
    f38 = r.getrandbits(8) | 1

    def setup(m):
        m.write(TOUCH, tch); m.w8(TOUCH + 0x15, 0); m.w8(SYS + 0x38, f38)
        m.acq = [0, 0]

    def rd(m, a):
        m.acq[0] += 1
        return 0xFF if m.acq[0] >= done_at else 0xFE

    def svc(m, a):
        m.acq[1] += 1
        if m.acq[1] >= touch_at:
            m.w8(TOUCH + 0x15, 1)
    stubs = (Stub("fpga_cmd", 0x4004927C, lambda m, a: None, 1), Stub("fpga_rd", 0x400491B8, rd, 0),
             Stub("runtime_event_service", 0x40049B08, svc, 0))
    return Case([], setup, ret=V, stubs=stubs)


TESTS[0x4003FCC0] = t_wait


def t_class(r):
    rate = r.choice([1, 10, 100, 4750, 1000, r.randrange(1, 10 ** 6)])
    k = r.randrange(1, 5)
    p1 = (22000000 // rate) * k
    p2 = (250000000 // rate) * k
    span = r.choice([p1 - 1, p1, p1 + 1, p2 - 1, p2, p2 + 1, 0, r.getrandbits(64), r.randrange(0, 2 * p2 + 2)]) & (2 ** 64 - 1)
    scale = r.choice([0.0, 1.0, 22.9, 23.0, 23.5, 100.0, -5.0, 0.37])
    n55c, n554 = r.choice([(1, 1), (23, 1), (46, 2), (100, 4), (r.getrandbits(16), r.randrange(1, 1000))])
    div = r.randrange(1, 1000)
    amp = r.choice([24 * div, 25 * div, 25 * div - 1, r.getrandbits(32)])
    ch = r.choice([0, 1, 2, 0xFFFFFFFF])
    ov = {0x524: (4, rate), 0x1B1: (1, k), 0x55C: (4, n55c), 0x554: (4, n554), 0x1C0: (4, div),
          0xB68: (4, amp), 0xB90: (4, amp), 0xB70: (4, f32(scale)), 0xB98: (4, f32(scale)),
          0xB78: (8, span), 0xBA0: (8, span ^ (r.getrandbits(1) if r.random() < 0.2 else 0))}
    return Case([ch], sys_bytes(r, 0x500, 0xC00, ov), ret=V)


TESTS[0x4003FD10] = t_class


# ================================================================ batch 15: serialiser, code page, strlen, USB veneers
SER_CUR, SER_MODE, SER_IMG = 0x4091F38C, 0x4091F390, 0x408A526C


def ser_setup(r, mode=None):
    img = bytes(r.getrandbits(8) for _ in range(0x220))
    src = bytes(r.getrandbits(8) for _ in range(16))
    cur = r.randrange(0, 0x200)
    md = r.choice([0, 1, 2]) if mode is None else mode

    def setup(m):
        m.write(SER_IMG, img); m.write(BUF, src); m.w32(SER_CUR, cur); m.w8(SER_MODE, md)
    return setup


SIZES = [1, 2, 4, 8, 1, 2, 4, 8, 0, 3, 16]
TESTS[0x40067134] = lambda r: Case([], ser_setup(r), ret=V)
TESTS[0x40067144] = lambda r: Case([], ser_setup(r), ret=V)
TESTS[0x40067154] = lambda r: Case([], ser_setup(r), ret=R)
TESTS[0x40067160] = lambda r: Case([r.randrange(0, 0x200), BUF, r.choice(SIZES)], ser_setup(r), ret=V)
TESTS[0x400672A8] = lambda r: Case([r.randrange(0, 0x200), BUF, r.choice(SIZES)], ser_setup(r), ret=V)
TESTS[0x40067278] = lambda r: Case([BUF, r.choice(SIZES)], ser_setup(r), ret=V)
TESTS[0x40067420] = lambda r: Case([BUF, r.choice(SIZES)], ser_setup(r), ret=V)
TESTS[0x40067450] = lambda r: Case([BUF, r.choice(SIZES)], ser_setup(r), ret=V,
                                   stubs=(rec("enc", 0x40067278, 2), rec("dec", 0x40067420, 2)))
CPS = [437, 437, 437, 850, 0, 437 + 65536]
TESTS[0x40069D90] = lambda r: Case([r.choice([r.randrange(0x80), r.randrange(0x80, 0x100), r.randrange(0x80, 0x100), 0x100, 0x7F, 0x80, 0xFF,
                                              r.getrandbits(32), r.getrandbits(16)]), r.choice(CPS)], ret=R)
TESTS[0x40069DF4] = lambda r: Case([r.choice([r.randrange(0x80), r.randrange(0x80, 0x100), 0x7F, 0x80, 0xFF, 0x100, -1 & 0xFFFFFFFF,
                                              r.getrandbits(32), r.getrandbits(9)]), r.choice(CPS)], ret=R)
TESTS[0x40069E38] = lambda r: Case([r.choice([r.randrange(0x61, 0x7B), r.randrange(0x80, 0x600), r.randrange(0x1D00, 0x2200),
                                              r.randrange(0x2C00, 0x2D30), r.randrange(0x2D00, 0x2D26), r.randrange(0xA640, 0xA800), r.randrange(0xFF00, 0x10000),
                                              r.getrandbits(16), r.getrandbits(32), 0, 0xFFFF, 0x10000, 0x1000, 0xFFF])], ret=R)


def t_strlen(r):
    n = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, r.randrange(0, 300)])
    off = r.randrange(8)
    data = bytes(r.randrange(1, 256) for _ in range(n)) + b"\0" + bytes(r.randrange(1, 256) for _ in range(16))
    return Case([BUF + off], lambda m: m.write(BUF + off, data), ret=R)


TESTS[0x4006A6A8] = t_strlen
SDSTATE = 0x40095490


def t_disk(tgt):
    def mk(r):
        st = r.choice([0, 1, 1, 2, 0xFFFFFFFF])
        return Case([r.getrandbits(32) for _ in range(4)], seeded(r.getrandbits(32), lambda m: m.w32(SDSTATE, st)), ret=R,
                    stubs=(rec("io", tgt, 4, ret=lambda m, a: m.srng.getrandbits(32)),))
    return mk


TESTS[0x4006A964] = t_disk(0x4006A738)
TESTS[0x4006A978] = t_disk(0x4006A868)
TESTS[0x40069018] = lambda r: Case([], ret=V, stubs=(rec("a98c", 0x4006A98C, 0), rec("usb_device_init", 0x4006B6A4, 1)))
TESTS[0x40069034] = lambda r: Case([], ret=V, stubs=(rec("usb_device_exit", 0x4006B728, 0), rec("disk_remount", 0x40049B90, 0)))
TESTS[0x40068E74] = lambda r: Case([], seeded(r.getrandbits(32)), ret=R,
                                   stubs=(rec("readl_c", 0x40068E64, 1, ret=lambda m, a: m.srng.getrandbits(32)),))


def t_touch(r):
    w = [r.choice([0, 0, 1, -1, 9, 10, -10, -11, 32767, -32768, r.getrandbits(16)]) & 0xFFFF for _ in range(4)]
    flag = r.choice([0, 1, 5])
    tail = bytes(r.getrandbits(8) for _ in range(0x10))

    def setup(m):
        for i, v in enumerate(w):
            m.write(TOUCH + 2 * i, v.to_bytes(2, "little"))
        m.write(TOUCH + 8, tail); m.w8(SYS + 0x671, flag)
    return Case([], setup, ret=V)


TESTS[0x40069CF8] = t_touch
TESTS[0x4006904C] = t_touch


# ================================================================ batch 16: touch controls, complex helpers, LRADC, strcmp
TSTATE, OBJ_A, OBJ_B, EN_B = 0x42E68998, 0x42E68668, 0x42E180C8, 0x42E22D28


def pos(r):
    return r.choice([0, 0, r.randrange(0, 800), r.randrange(0, 800), r.randrange(-5, 5), r.getrandbits(16)]) & 0xFFFF


def s32(v):
    return v - (1 << 32) if v & 0x80000000 else v


def touch_ctl(r):
    tp = [pos(r) for _ in range(4)]
    if r.random() < 0.15:
        tp[2] = tp[3] = 0
    st = bytearray(r.getrandbits(8) for _ in range(0x80))
    for o in (4, 0x60, 0x74):
        _struct.pack_into("<I", st, o, r.choice([0, 5, 20, 60, 0xFFFF, 0x7FFFFFFF, r.randrange(0, 400), r.getrandbits(32)]))
    a = bytearray(r.getrandbits(8) for _ in range(0xB8))
    for o in (0x5C, 0x60, 0xB0):
        _struct.pack_into("<I", a, o, r.choice([r.randrange(0, 800), r.randrange(0, 40), r.getrandbits(32)]))
    _struct.pack_into("<I", a, 0x58, r.choice([0, 1, r.getrandbits(32)]))
    a[3] = r.choice([0, 1, 7])
    b0 = r.randrange(0, 60) | (r.getrandbits(16) if r.random() < 0.1 else 0)
    bw = [r.choice([r.randrange(0, 800), r.getrandbits(32), 0x7FFFFFF0]) for _ in range(2)]
    b46 = r.choice([0, 1])
    en = r.choice([0, 1, 9])
    ov = {4: (2, r.choice([0, 30, r.getrandbits(16)])), 6: (2, r.choice([0, 30, r.getrandbits(16)])), 0xA: (2, r.choice([0, 480, 300, r.getrandbits(16)])),
          0x2AA: (1, r.choice([0, 1])), 0x671: (1, r.choice([0, 1, 4])),
          0x808: (4, r.randrange(0, 800)), 0x80C: (4, r.randrange(0, 800)), 0x810: (4, r.choice([r.randrange(0, 600), r.getrandbits(32)])),
          0x814: (4, r.choice([r.randrange(0, 600), -r.randrange(1, 600)]))}
    # exact boundaries: ties between the two marks, limits equal to a distance, y on the third line
    def s16(v):
        v &= 0xFFFF
        return v - 0x10000 if v & 0x8000 else v
    x, y = s16(tp[2]), s16(tp[3])
    mode = r.random()
    if mode < 0.15:
        a[0x60:0x64] = a[0x5C:0x60]
        bw[1] = bw[0]
    elif mode < 0.5:
        o = _struct.unpack_from("<I", a, 0xB0)[0]
        da = [abs(x - ((_struct.unpack_from("<I", a, q)[0] + o) & 0xFFFF)) for q in (0x5C, 0x60)]
        _struct.pack_into("<I", st, 0x74, r.choice(da))
        db = [abs(x - s32((w + b0) & 0xFFFFFFFF)) & 0xFFFFFFFF for w in bw]
        _struct.pack_into("<I", st, 0x60, r.choice(db))
    if r.random() < 0.3:
        tp[3] = (ov[6][1] + ((ov[0xA][1] * 0xAAAB) >> 17) + r.choice([0, 0, -1, 1])) & 0xFFFF
    sb = sys_bytes(r, 0, 0x820, ov)
    xb = [r.choice([r.randrange(0, 800), r.getrandbits(32)]) for _ in range(2)]

    def setup(m):
        sb(m)
        for i, v in enumerate(tp):
            m.write(TOUCH + 2 * i, v.to_bytes(2, "little"))
        m.write(TSTATE, bytes(st)); m.write(OBJ_A, bytes(a))
        m.write(OBJ_B, (b0 & 0xFFFF).to_bytes(2, "little")); m.w8(OBJ_B + 0x46, b46)
        m.w32(OBJ_B + 0xAC64, bw[0]); m.w32(OBJ_B + 0xAC68, bw[1]); m.w8(EN_B, en)
        m.w32(SYS + 0x3DA534, xb[0]); m.w32(SYS + 0x3DA538, xb[1])
    return setup


for _a in (0x40069084, 0x400691D4, 0x40069324, 0x400690E4, 0x40069234, 0x40069384, 0x40069524, 0x40069428):
    TESTS[_a] = lambda r: Case([], touch_ctl(r), ret=V)
TESTS[0x40069150] = lambda r: Case([], touch_ctl(r), ret=V)
TESTS[0x400692A0] = lambda r: Case([], touch_ctl(r), ret=V)


def t_cmag(r):
    n = r.choice([0, 1, 2, 5, 17, -1 & 0xFFFFFFFF])
    vals = [r.choice([0.0, 1.0, -3.5, 1e-20, 1e20, r.uniform(-1000, 1000)]) for _ in range(40)]
    src = b"".join(_struct.pack("<f", v) for v in vals)
    return Case([BUF, BUF + 0x200, n], lambda m: (m.write(BUF, src), m.write(BUF + 0x200, bytes(80))), ret=V)


TESTS[0x40066538] = t_cmag


def t_cmul(r):
    v = [f32(r.choice([0.0, 1.0, -2.25, 1e30, r.uniform(-100, 100)])) for _ in range(4)]
    c = Case([], ret="d")
    c.dregs = {0: v[0] | (v[1] << 32), 1: v[2] | (v[3] << 32)}
    return c


TESTS[0x40066584] = t_cmul
TESTS[0x40068E94] = lambda r: Case([], seeded(r.getrandbits(32)), ret=V,
                                   stubs=(rec("readl_c", 0x40068E64, 1, ret=lambda m, a: m.srng.getrandbits(32)), rec("writel_c", 0x40068E6C, 2)))


def t_strcmp(r):
    n = r.randrange(0, 40)
    s1 = bytearray(r.randrange(1, 256) for _ in range(n))
    s2 = bytearray(s1)
    k = r.random()
    if k < 0.3 and n:
        s2[r.randrange(n)] = r.randrange(1, 256)
    elif k < 0.45:
        s2 += bytes([r.randrange(1, 256)])
    elif k < 0.55 and n:
        s2 = s2[:r.randrange(n)]
    o1, o2 = r.randrange(4), r.randrange(4)
    return Case([BUF + o1, BUF + 0x100 + o2], lambda m: (m.write(BUF + o1, bytes(s1) + b"\0" + b"\x55" * 8),
                                                        m.write(BUF + 0x100 + o2, bytes(s2) + b"\0" + b"\xAA" * 8)), ret=R)


TESTS[0x40068F78] = t_strcmp


def t_geom(r):
    ss = r.choice([512, 1, 4096, r.randrange(1, 70000)])
    st = r.choice([0, 1, 1, 2])
    cap = r.getrandbits(64) if r.random() < 0.5 else r.getrandbits(40)
    sec = r.getrandbits(32)

    def setup(m):
        m.w32(SECTOR_SIZE, ss); m.w32(SDSTATE, st); m.write(0x42E68C60, cap.to_bytes(8, "little")); m.w32(SD_INFO + 4, sec)
    return Case([], setup, ret=V)


TESTS[0x4006A98C] = t_geom


# ================================================================ batch 17: acquisition pipeline
FFE4 = rec("timebase_ffe4", 0x4003FFE4, 1)
TESTS[0x40040030] = lambda r: Case([], sys_bytes(r, 0x200, 0x300), ret=V, stubs=(FFE4,))
TESTS[0x4004003C] = lambda r: Case([], sys_bytes(r, 0x200, 0x300), ret=V, stubs=(FFE4,))
TESTS[0x40040048] = lambda r: Case([], sys_bytes(r, 0x200, 0x300, {0x2B0: (4, r.choice([0, 1, 10, 4750, 0xFFFFFFFF, r.getrandbits(32)]))}),
                                   ret=V, stubs=(FFE4,))


def wcr_stub():
    def fn(m, a):
        q = regs3(m)
        m.calls.append(("workspace_copy_and_resampling", q[0], q[1], q[2], q[3], m.r32(m.reg(A.UC_ARM_REG_SP))))
    return Stub("wcr*", 0x4005C7BC, fn, 0)


def t_postproc(r):
    ov = {0x3E: (1, r.choice([0, 1])), 0x102: (1, r.choice([0, 1])), 0x554: (4, r.choice([1, 5, 100])), 0x55C: (4, r.choice([1, 5, 100, 4]))}
    return Case([r.choice([0, 1, 2, 7])], sys_bytes(r, 0, 0x800, ov), ret=V,
                stubs=(rec("tb13", 0x40040048, 0), rec("amp", 0x4003FD10, 1), wcr_stub(), rec("final", 0x4003FE14, 1)))


TESTS[0x4004006C] = t_postproc
TESTS[0x400405E0] = fcase(lambda r: [], state=lambda r: (sys_bytes(r, 0x600, 0x700), None), extra=(rec("fa8", 0x40065FA8, 1),))


def t_capblock(r):
    pre = r.choice([0, 0, 1])
    base = sys_bytes(r, 0x600, 0x800, {0x70E: (1, pre)})

    def pe0_set(m, a):
        m.w8(SYS + 0x70E, 1)
    stubs = (rec("trig", 0x4003FAB8, 0), rec("sel1f", 0x4003FA94, 1), rec("f094", 0x40049094, 1), rec("csi_buf", 0x40066030, 1),
             rec("timer_stop", 0x40042C40, 1), rec("csi_on", 0x400660AC, 0), Stub("pe0_set", 0x4003F9F8, pe0_set, 0),
             rec("pe0_clr", 0x4003FA04, 0), rec("timer_start", 0x40042C1C, 1), rec("f108", 0x40066108, 0))
    return Case([r.getrandbits(32), r.getrandbits(32)], base, ret=V, stubs=stubs)


TESTS[0x40040638] = t_capblock
CAPB = rec("capture_block", 0x40040638, 2)
TESTS[0x400406A4] = lambda r: Case([r.getrandbits(32) for _ in range(3)], sys_bytes(r, 0x600, 0x700), ret=V, stubs=(CAPB,))
TESTS[0x400406C8] = lambda r: Case([r.choice([0, 1, 7, 0x7FFF, 0x8000, 0xFFFF, r.getrandbits(32)])], ret=V, stubs=(CAPB,))
for _a in (0x400406DC, 0x400406FC, 0x4004071C):
    TESTS[_a] = lambda r: Case([], ret=V, stubs=(rec("capture_bank", 0x400406C8, 1),))


def t_step(r):
    p_ready = r.choice([0.3, 0.9])
    ov = {0x3E: (1, r.choice([0, 1])), 0x102: (1, r.choice([0, 1])), 0x100: (1, r.choice([0, 1, 2, 3])),
          0x6EE: (1, 0), 0x6F6: (2, r.randrange(0, 3000)), 0x6FC: (4, r.randrange(0, 5000)), 0x704: (4, r.randrange(0, 5000)),
          0x7B4: (4, 0x41000000 + r.randrange(0, 0x10000)), 0x7B8: (4, 0x41100000 + r.randrange(0, 0x10000))}
    base = sys_bytes(r, 0, 0x800, ov)

    def setup(m):
        base(m); m.w8(TOUCH + 0x15, 0)

    def ready(m, a):
        m.w8(SYS + 0x6EE, 1 if m.srng.random() < p_ready else 0)

    def svc(m, a):
        if m.srng.random() < 0.3:
            m.w8(TOUCH + 0x15, 1)
    stubs = (Stub("ready", 0x4003F8B8, ready, 0), Stub("svc", 0x40049B08, svc, 0), rec("tb", 0x4004003C, 0), rec("geom", 0x400405E0, 0),
             rec("all", 0x400406DC, 0), rec("b03", 0x400406FC, 0), rec("b47", 0x4004071C, 0),
             rec("d74", 0x40040D74, 2), rec("e44", 0x40040E44, 2), rec("bec", 0x40040BEC, 2), rec("copy_u16", 0x40035728, 3))
    return Case([], seeded(r.getrandbits(32), setup), ret=V, stubs=stubs)


TESTS[0x400417E4] = t_step
TESTS[0x40041918] = lambda r: Case([], ret=V, stubs=(rec("full", 0x400413C8, 0),))


def t_sweep(r):
    tb = bytearray(r.getrandbits(8) for _ in range(0x20))
    tb[0x18] = r.choice([0, 1, 2, 3, 4, 5, 0x13, 0x12, 6])
    for o in (0x17, 0x15, 0x0D):
        tb[o] = r.choice([0, 0, 0, 1])
    n = r.randrange(0, 6)
    ov = {8: (2, n), 0x700: (4, r.randrange(0, 4)), 0x38: (1, r.getrandbits(8))}
    base = sys_bytes(r, 0, 0x800, ov)

    def setup(m):
        base(m); m.write(TOUCH, bytes(tb))

    def adv(m, a):
        m.w32(SYS + 0x700, m.r32(SYS + 0x700) + 1)

    def svc(m, a):
        if m.srng.random() < 0.2:
            m.w8(TOUCH + 0x15, 1)
    stubs = (rec("rearm", 0x4003C520, 0), rec("step", 0x400417E4, 0), rec("commit", 0x4003BEB4, 0), rec("pcp", 0x4003C9F4, 0),
             Stub("adv", 0x4003BF98, adv, 0), rec("w6f0", 0x4003F910, 0), rec("p08", 0x4003F8E0, 0), Stub("svc", 0x40049B08, svc, 0))
    return Case([], seeded(r.getrandbits(32), setup), ret=V, stubs=stubs)


TESTS[0x4004191C] = t_sweep


def t_disp(r):
    tb = bytearray(r.getrandbits(8) for _ in range(0x20))
    tb[0x17] = r.choice([0, 0, 0, 1])
    ov = {0x3C: (1, r.choice([0, 0, 1])), 0x28: (1, r.choice([0, 1])), 0x38: (1, r.getrandbits(8))}
    base = sys_bytes(r, 0, 0x100, ov)
    return Case([], lambda m: (base(m), m.write(TOUCH, bytes(tb))), ret=V,
                stubs=(rec("full", 0x40041918, 0), rec("sweep", 0x4004191C, 0), rec("commit", 0x4003BEB4, 0)))


TESTS[0x400419CC] = t_disp


# ================================================================ batch 18: CSI bank unpacking
BANKS, CALOBJ = 0x42DF76D4, 0x42E53A0C


def conv_state(r):
    bank = bytes(r.getrandbits(8) for _ in range(8 * 0x2134 + 0x1400))
    cal = b"".join(_struct.pack("<i", r.choice([1000, 990, 1012, r.randrange(500, 1500), r.randrange(-3000, 3000), r.getrandbits(31)])) for _ in range(16))
    ov = {0x41: (1, r.randrange(13)), 0x105: (1, r.randrange(13)), 0x1C0: (4, r.choice([1, 2, 10, r.randrange(1, 100)])),
          0x60: (4, r.randrange(0, 300)), 0x120: (4, r.randrange(0, 300)), 0xE0: (4, r.choice([32, 31, 40, r.randrange(0, 100), r.getrandbits(32)])),
          0x1A0: (4, r.choice([32, 33, r.randrange(0, 100)])), 0x1B8: (1, r.getrandbits(8)), 0xA: (2, r.choice([480, 400, 0, r.getrandbits(16)])),
          0x264: (4, r.randrange(0, 32)), 0x6F2: (2, r.randrange(0, 0x1000))}
    for i in range(13):
        ov[0x1C4 + 4 * i] = (4, r.choice([r.randrange(1, 5000), r.getrandbits(32)]))
        ov[0x214 + 4 * i] = (4, r.choice([r.randrange(1, 5000), r.getrandbits(32)]))
    sb = sys_bytes(r, 0, 0x300, ov)

    def setup(m):
        sb(m); m.write(BANKS, bank); m.write(CALOBJ + 0x10, cal)
        m.write(SYS + 0x6F2, _struct.pack("<H", ov[0x6F2][1]))
    return setup


def t_conv(r):
    n = r.choice([0, 1, 2, 100, r.randrange(0, 600), 0x10000 + 3])
    stack = [r.choice([0, 3, 7]) | (r.getrandbits(16) << 16 if r.random() < 0.2 else 0), r.randrange(0, 256), n]
    return Case([r.randrange(8), r.choice([0, 1]), BUF, r.choice([4, 8, 0x10004])], conv_state(r), ret=V, stack=stack)


TESTS[0x4004073C] = t_conv


def conv_stub():
    def fn(m, a):
        q = regs3(m)
        sp = m.reg(A.UC_ARM_REG_SP)
        m.calls.append(("conv", q[0], q[1], q[2], q[3], m.r32(sp), m.r32(sp + 4), m.r32(sp + 8)))
    return Stub("conv*", 0x4004073C, fn, 0)


for _a in (0x40040BEC, 0x40040D74, 0x40040E44):
    TESTS[_a] = lambda r: Case([BUF, r.choice([0, 1])], sys_bytes(r, 0x600, 0x700), ret=V, stubs=(conv_stub(),))
for _a in (0x400408FC, 0x40040A5C, 0x40040B20):
    TESTS[_a] = lambda r: Case([BUF, r.choice([0, 1]), r.choice([0, 5, 0x10005, r.getrandbits(32)])], sys_bytes(r, 0x600, 0x700), ret=V,
                               stubs=(conv_stub(),))
