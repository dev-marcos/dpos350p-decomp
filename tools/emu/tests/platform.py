"""Behavioural tests for the platform drivers (GIC/IRQ, UART + printf, SPI, SD/MMC, CSI)."""
import random as _random

from unicorn import arm_const as A

from tools.emu.emu import Stub
from tools.emu.tests.fatfs import Case

N = 200
BUF = 0x41000000
GIC = 0x01C80000
G_GIC, IRQ_TABLE = 0x42E68BF0, 0x42E67954
HANDLER = 0x41F00000          # fake handler address (stubbed)
V, R = 0, 32


def regs(rng, base, n_words):
    vals = [rng.getrandbits(32) for _ in range(n_words)]
    return lambda m: [m.w32(base + 4 * i, v) for i, v in enumerate(vals)]


def compose(*fns):
    return lambda m: [f(m) for f in fns]


def rec(name, addr, nargs, ret=None):
    return Stub(name, addr, (lambda m, a: ret), nargs)


def _printf_call(name):
    """Record the format pointer plus only the register arguments its conversions consume
    (leftover values in unused argument registers are not behaviour)."""
    def fn(m, a):
        fmt = m.reg(A.UC_ARM_REG_R0)
        raw = bytes(m.read(fmt, 128)).split(b"\0")[0] if 0x40000000 <= fmt < 0x44000000 else b""
        n = raw.replace(b"%%", b"").count(b"%")
        regs_ = [m.reg(x) for x in (A.UC_ARM_REG_R1, A.UC_ARM_REG_R2, A.UC_ARM_REG_R3)]
        m.calls.append((name, fmt, tuple(regs_[:min(n, 3)])))
    return Stub(name + "*", {"dbg_printf": 0x4006709C, "debug_printf_stub": 0x4006711C}[name], fn, 0)


PRINTF = (_printf_call("dbg_printf"), _printf_call("debug_printf_stub"))


def chars_stub():
    """console_putc replaced by a recorder (the order of printed characters is compared)."""
    return rec("console_putc", 0x40066CF0, 1)


def strings(rng):
    pool = [b"hello", b"", b"x", b"DPOS350P", b"a" * 30, bytes(rng.randrange(1, 127) for _ in range(12))]
    return b"".join(s + b"\0" for s in pool)


TESTS = {}

# ---------------------------------------------------------------- GIC / IRQ
def gic_state(rng, irq_base=None):
    ib = rng.choice([0, 0, 32]) if irq_base is None else irq_base
    init = compose(regs(rng, GIC + 0x1000, 0x400), regs(rng, GIC + 0x2000, 0x40))

    def setup(m):
        init(m)
        m.w32(G_GIC, GIC); m.w32(G_GIC + 4, 0); m.w32(G_GIC + 8, ib); m.w32(G_GIC + 12, 0x9C)
        for i in range(0x9C):
            m.w32(IRQ_TABLE + 4 * i, HANDLER)
    return setup


def gic_case(args_fn, ret=V, **kw):
    def mk(r):
        return Case(args_fn(r), gic_state(r, **kw), ret=ret, stubs=PRINTF + (rec("irq_handler", HANDLER, 0),))
    return mk


TESTS[0x400668D0] = lambda r: Case([GIC + 0x2000 + 4 * r.randrange(8)], regs(r, GIC + 0x2000, 8), ret=R)
TESTS[0x400668D8] = lambda r: Case([GIC + 0x1000 + 4 * r.randrange(8), r.getrandbits(32)], ret=V)
TESTS[0x400668E0] = gic_case(lambda r: [])
TESTS[0x400668E4] = gic_case(lambda r: [])
TESTS[0x40066950] = gic_case(lambda r: [G_GIC, r.randrange(0x9C)])
TESTS[0x40066994] = gic_case(lambda r: [G_GIC, r.randrange(0x9C)])
TESTS[0x400669D8] = gic_case(lambda r: [], R)
TESTS[0x400669E0] = gic_case(lambda r: [GIC])
TESTS[0x40066B10] = gic_case(lambda r: [GIC])
TESTS[0x40066B8C] = gic_case(lambda r: [])
TESTS[0x40066BD4] = gic_case(lambda r: [r.randrange(0x9C), r.choice([HANDLER, 0x40042C64, r.getrandbits(32)]),
                                        r.choice([0, 1, 5, 0x1F, r.getrandbits(8)]), r.choice([0, 1, 2])])
TESTS[0x40066C6C] = gic_case(lambda r: [])
TESTS[0x40066B88] = gic_case(lambda r: [])

# ---------------------------------------------------------------- UART / printf
UART = (0x01C28000, 0x01C28400, 0x01C28800)


def usr_model(seed):
    rng = _random.Random(seed)

    def dev(m, op, addr, size, value):
        if op == "R" and (addr - 0x7C) in UART:
            return rng.getrandbits(32) & ~2 | (2 if rng.random() < 0.6 else 0)
        return None
    return dev


def uart_state(rng, port=None):
    seed = rng.getrandbits(32)
    p = rng.choice([0, 1, 2]) if port is None else port
    data = strings(rng)
    init = compose(regs(rng, 0x01C28000, 0x300), regs(rng, 0x01C20060, 4))

    def setup(m):
        init(m)
        m.w32(0x42E68CDC, p)
        m.write(BUF, data)
        m.devices.append((0x01C28000, 0x01C28C00, usr_model(seed)))
    return setup


def ucase(args_fn, ret=V, stubs=()):
    def mk(r):
        return Case(args_fn(r), uart_state(r), ret=ret, stubs=tuple(stubs))
    return mk


TESTS[0x40066C80] = lambda r: Case([0x01C2006C], regs(r, 0x01C20060, 4), ret=R)
TESTS[0x40066C88] = lambda r: Case([0x01C28000 + 4 * r.randrange(8), r.getrandbits(32)], ret=V)
TESTS[0x40066C90] = ucase(lambda r: [r.choice([0, 1, 2]), r.getrandbits(8)])
TESTS[0x40066CF0] = ucase(lambda r: [r.getrandbits(8)])
STR_ADDRS = lambda r: BUF + r.choice([0, 6, 7, 9, 18, 49])
TESTS[0x40066D04] = ucase(lambda r: [STR_ADDRS(r)], stubs=[chars_stub()])
TESTS[0x40066D28] = ucase(lambda r: [r.choice([0x20, 0x30, r.getrandbits(8)]), r.choice([0, 1, 2, 7, 40])], stubs=[chars_stub()])
TESTS[0x40066D54] = ucase(lambda r: [STR_ADDRS(r), r.choice([-1, 0, 1, 3, 8])], stubs=[chars_stub()])


def num_args(r):
    val = r.choice([0, 1, 9, 10, 999, 1000, 1234567, -1, -1000, 0x7FFFFFFF, -0x80000000, r.getrandbits(32)])
    base = r.choice([-10, 10, 16, 8, 3, 2, -16, 0])
    width = r.choice([0, 1, 4, 8, 12, 80, 81, -5, -12, r.randrange(-100, 100)])
    pad = r.choice([0, 0x20, 0x30, r.getrandbits(8)])
    return [val, base, width, pad]


TESTS[0x40066D88] = ucase(num_args, stubs=[chars_stub()])
FMT = [b"d", b"5d", b"05d", b"-5d", b"x", b"08X", b"o", b"u", b"c", b"3c", b"-3c", b"s", b"10s", b"-10s", b"%",
       b"q", b"", b"12i", b"-", b"0", b"7z"]


def conv_args(r):
    return [BUF + 0x200, r.choice([BUF + r.choice([0, 6, 9]), r.getrandbits(32), r.choice([0, 1, 65, 1000, -7])])]


def conv_state(r):
    base = uart_state(r)
    fmt = r.choice(FMT) + b"rest\0"

    def setup(m):
        base(m)
        m.write(BUF + 0x200, fmt)
    return setup


TESTS[0x40066ED0] = lambda r: (lambda st, a, f: Case(a, st, ret=R, stubs=(chars_stub(),)))(
    conv_state(r), [BUF + 0x200, None], None) if False else _conv_case(r)


def _conv_case(r):
    fmt = r.choice(FMT)
    s_arg = fmt.rstrip(b"0123456789-").endswith(b"s")
    arg = BUF + r.choice([0, 6, 9]) if s_arg else r.choice([0, 1, 65, 1000, -7, r.getrandbits(32)]) & 0xFFFFFFFF
    base = uart_state(r)

    def setup(m):
        base(m)
        m.write(BUF + 0x200, fmt + b"rest\0")
    return Case([BUF + 0x200, arg], setup, ret=R, stubs=(chars_stub(),))


TESTS[0x40066ED0] = _conv_case


def _printf_case(r):
    fmts = [b"plain text", b"v=%d|%5d|%-5d|", b"%x %08X %o %u", b"%s and %10s!", b"%c%c%%", b"%q %d", b"a%",
            b"%d,%d,%d,%d,%d"]
    f = r.choice(fmts) + b"\0"
    args = [BUF + 0x300] + [r.choice([0, 7, -42, 123456, BUF + 6, 0x41]) & 0xFFFFFFFF for _ in range(3)]
    stack = [r.choice([1, 2, 3, BUF + 9]) for _ in range(4)]
    base = uart_state(r)

    def setup(m):
        base(m)
        m.write(BUF + 0x300, f)
        m.w32(0x01C2006C, r_gate)
    r_gate = r.choice([0xFFFFFFFF, 0, 1 << (16 + 0), r.getrandbits(32)])
    return Case(args, setup, ret=V, stubs=(chars_stub(),), stack=stack)


TESTS[0x4006709C] = _printf_case
TESTS[0x4006711C] = lambda r: Case([BUF, 1, 2, 3], ret=V)
TESTS[0x40067128] = lambda r: Case([r.getrandbits(32)], ret=V)

# ---------------------------------------------------------------- SPI
SPI0 = 0x01C68000


def spi_model(seed, p):
    rng = _random.Random(seed)

    def dev(m, op, addr, size, value):
        if op != "R":
            return None
        off = addr - SPI0
        if off in (0x04, 0x08):                     # self-clearing reset / XCH bits
            cur = int.from_bytes(m.uc.mem_read(addr, 4), "little")
            return cur & ~0x80000000 | (0x80000000 if rng.random() < p else 0)
        if off == 0x1C:                             # FIFO status: random fill levels
            return rng.randrange(65) | (rng.randrange(65) << 16)
        return None
    return dev


def spi_state(r):
    seed, p = r.getrandbits(32), r.choice([0.0, 0.5, 0.9])
    data = bytes(r.getrandbits(8) for _ in range(512))
    init = compose(regs(r, SPI0, 0x100), regs(r, 0x01C20000, 0x100), regs(r, 0x01C20800, 0x40))
    words = {0x42E68CC4: r.choice([0, 1, 2]), 0x42E68CC8: r.choice([0, 1]), 0x42E68CCC: r.choice([0, 1]),
             0x42E68CD0: r.choice([0, 0, 1]), 0x4009549C: r.choice([1000000, 24000000, 50000000, 100000000])}
    ports = [0x01C20890, 0x01C208B4]

    def setup(m):
        init(m)
        m.write(BUF, data)
        for a, v in words.items():
            m.w32(a, v)
        for i in range(2):
            m.w32(0x42E68C20 + 4 * i, ports[i]); m.w32(0x42E68C2C + 4 * i, r_pins[i])
        m.devices.append((SPI0, SPI0 + 0x1000, spi_model(seed, p)))
    r_pins = [r.randrange(25), r.randrange(25)]
    return setup


def scase(args_fn, ret=V, extra=()):
    def mk(r):
        return Case(args_fn(r), spi_state(r), ret=ret, stubs=PRINTF + tuple(extra))
    return mk


TESTS[0x40042F00] = scase(lambda r: [0])
TESTS[0x4004302C] = scase(lambda r: [0])
TESTS[0x40043088] = scase(lambda r: [0])
TESTS[0x400430E8] = scase(lambda r: [0, r.choice([0, 1, 31, 0x1000000, r.getrandbits(32)]), r.choice([0, 5, r.getrandbits(32)])])
TESTS[0x40043138] = scase(lambda r: [0])


def xfer_args(r):
    n = r.choice([0, 1, 5, 31, 32, 33, 64, 100])
    tx = r.choice([0, BUF])
    rx = BUF + 0x400 if (n % 32) else r.choice([0, BUF + 0x400])     # the byte tail has no NULL check
    return [0, tx, rx, n]


TESTS[0x40043164] = scase(xfer_args, R)
XFER_STUB = (rec("sys_spi_cpu_transfer", 0x40043164, 4),)
TESTS[0x40043330] = scase(lambda r: [0, BUF, BUF + 0x400, r.choice([0, 4, 0xFFFFFF, 0x1000000, -1])], R, XFER_STUB)
TESTS[0x40043360] = lambda r: Case([0, BUF, r.choice([0, 4, 0x1000000]), BUF + 0x400], spi_state(r), ret=R,
                                   stubs=PRINTF + XFER_STUB, stack=[r.choice([0, 8, 0x1000000])])
TESTS[0x400433C8] = lambda r: Case([0, BUF, r.choice([0, 4, 0x1000000]), BUF + 0x40], spi_state(r), ret=R,
                                   stubs=PRINTF + XFER_STUB, stack=[r.choice([0, 8, 0x1000000])])

# ---------------------------------------------------------------- CSI
CSI = 0x01CB4000
CSI_TBL = 0x42E68C50


def lock_model(seed):
    rng = _random.Random(seed)

    def dev(m, op, addr, size, value):
        if op == "R" and addr == 0x01C2002C:
            cur = int.from_bytes(m.uc.mem_read(addr, 4), "little")
            return cur & ~0x10000000 | (0x10000000 if rng.random() < 0.4 else 0)
        return None
    return dev


def csi_state(r):
    seed = r.getrandbits(32)
    init = compose(regs(r, CSI, 0x100), regs(r, 0x01C20000, 0x100), regs(r, 0x01C20800, 0x40))
    data = bytes(r.choice([0, 1, 2, 3, 4, 0xFF, 0x80, r.getrandbits(8)]) for _ in range(256))
    g = gic_state(r)

    def setup(m):
        init(m)
        g(m)
        m.write(BUF, data)
        m.w32(CSI_TBL, CSI); m.w32(CSI_TBL + 4, CSI + 0x1000)
        m.devices.append((0x01C2002C, 0x01C20030, lock_model(seed)))
    return setup


CSI_STUBS = PRINTF + (rec("dcache_clean_inv_all", 0x400632F8, 0), rec("csi_set_fmt", 0x40065D04, 3, 0),
                      rec("delay_us", 0x40042D0C, 1), rec("delay_ms", 0x40042CF0, 1), rec("irq_handler", HANDLER, 0))


def ccase(args_fn, ret=V):
    def mk(r):
        return Case(args_fn(r), csi_state(r), ret=ret, stubs=CSI_STUBS)
    return mk


reg = lambda r: CSI + 4 * r.randrange(0x40)
TESTS[0x4006582C] = ccase(lambda r: [reg(r)], R)
TESTS[0x40065834] = ccase(lambda r: [reg(r), r.getrandbits(32)])
TESTS[0x4006583C] = ccase(lambda r: [reg(r), r.getrandbits(32)])
TESTS[0x40065864] = ccase(lambda r: [reg(r), r.getrandbits(32)])
TESTS[0x4006588C] = ccase(lambda r: [reg(r), r.getrandbits(32)])
TESTS[0x40065890] = ccase(lambda r: [reg(r)], R)
TESTS[0x40065894] = ccase(lambda r: [reg(r), r.getrandbits(32), r.getrandbits(32)])
TESTS[0x400658BC] = ccase(lambda r: [r.choice([0, 1, 2, 3]), r.getrandbits(32)], R)
sel = lambda r: r.choice([0, 0, 1])
ch = lambda r: r.choice([0, 1, 2, 3])
for a in (0x400658DC, 0x400658EC, 0x40065C64, 0x40065C68):
    TESTS[a] = ccase(lambda r: [sel(r)])
for a in (0x400658FC, 0x40065954):
    TESTS[a] = ccase(lambda r: [sel(r), BUF + r.randrange(64)])
TESTS[0x400659D0] = ccase(lambda r: [sel(r), ch(r), BUF + r.randrange(64)])
TESTS[0x40065A60] = lambda r: Case([sel(r), ch(r), r.choice([0, 1, 2])], csi_state(r), ret=V, stubs=CSI_STUBS,
                                   stack=[r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])])
for a in (0x40065A98, 0x40065EA0):
    TESTS[a] = ccase(lambda r: [sel(r), r.choice([0, 1, 2, 3, 4, 5]), r.getrandbits(8)])
TESTS[0x40065AE8] = lambda r: Case([sel(r), ch(r), r.getrandbits(16), r.getrandbits(16)], csi_state(r), ret=V,
                                   stubs=CSI_STUBS, stack=[r.getrandbits(16), r.getrandbits(16)])
TESTS[0x40065B70] = ccase(lambda r: [sel(r), ch(r), r.getrandbits(16), r.getrandbits(16)])
for a in (0x40065BB8, 0x40065EA4, 0x40065C50, 0x40065EAC):
    TESTS[a] = ccase(lambda r: [sel(r), ch(r), r.getrandbits(32)])
for a in (0x40065BD4, 0x40065EA8):
    TESTS[a] = ccase(lambda r: [sel(r), ch(r), BUF + 0x100])
TESTS[0x40065C80] = ccase(lambda r: [sel(r), BUF + r.randrange(64)], R)
for a in (0x40065EB0, 0x40065EF8, 0x40065F48, 0x40066054, 0x400660AC, 0x400660C0, 0x400660C4, 0x40066108):
    TESTS[a] = ccase(lambda r: [])
TESTS[0x40065FA8] = ccase(lambda r: [r.getrandbits(32)], R)
TESTS[0x40066030] = ccase(lambda r: [r.getrandbits(32)])
TESTS[0x400660C8] = ccase(lambda r: [r.getrandbits(32)])
TESTS[0x4006610C] = ccase(lambda r: [r.getrandbits(8)])

# ---------------------------------------------------------------- SD/MMC (part 1)
MMC0 = 0x01C0F000


def mmc_model(seed, p_busy, p_err):
    rng = _random.Random(seed)

    def dev(m, op, addr, size, value):
        if op != "R":
            return None
        off = (addr - MMC0) & 0xFFF
        cur = int.from_bytes(m.uc.mem_read(addr, 4), "little")
        if off == 0x38:                         # RINTSTS: random done/error bits
            v = rng.getrandbits(32) & ~0xBFC2
            if rng.random() < p_err: v |= 1 << rng.choice([1, 6, 7, 8, 9, 15])
            return v
        if off in (0x18, 0x80):                 # self-clearing start / IDMAC reset bits
            bit = 0x80000000 if off == 0x18 else 1
            return cur & ~bit | (bit if rng.random() < p_busy else 0)
        if off == 0x3C:                         # STATUS: FIFO empty (2) / full (3)
            return rng.getrandbits(32) & ~0xC | (rng.choice([0, 4, 8, 12]) if rng.random() < p_busy else 0)
        return None
    return dev


def mmc_state(r, big=False):
    seed = r.getrandbits(32)
    pb, pe = r.choice([0.0, 0.5, 0.9]), r.choice([0.0, 0.0, 0.2])
    init = compose(regs(r, MMC0, 0xC00), regs(r, 0x01C20000, 0x100))
    data = bytes(r.getrandbits(8) for _ in range(0x2000))
    cap = r.choice([0, 1000, 0x100000, 0xFFFFFFFF, r.getrandbits(32)])
    blksz, blocks = r.choice([512, 4, 64]), r.choice([0, 1, 2, 8, 9, 300] + ([2048, 2049] if big else []))
    if big and r.random() < 0.4:                 # exactly 256 / 257 descriptors
        blksz, blocks = 512, r.choice([2048, 2049])
    flags = r.choice([0, 1, 2, 3])

    def setup(m):
        init(m)
        m.write(BUF, data)
        m.w32(0x42E68CC0, cap)
        m.w32(BUF + 0x3000, BUF + 0x100); m.w8(BUF + 0x3004, flags)
        m.w32(BUF + 0x3008, blksz); m.w32(BUF + 0x300C, blocks)
        m.devices.append((MMC0, MMC0 + 0x3000, mmc_model(seed, pb, pe)))
    return setup


MMC_STUBS = PRINTF + (rec("delay_us", 0x40042D0C, 1), rec("delay_ms", 0x40042CF0, 1))


def mcase(args_fn, ret=V, big=False):
    def mk(r):
        return Case(args_fn(r), mmc_state(r, big), ret=ret, stubs=MMC_STUBS)
    return mk


mn = lambda r: r.choice([0, 0, 1, 2])
TESTS[0x400675A8] = mcase(lambda r: [r.getrandbits(16)])
TESTS[0x400675AC] = mcase(lambda r: [r.getrandbits(16)])
TESTS[0x4006763C] = mcase(lambda r: [r.choice([0, 1, 2, 3, r.getrandbits(32)])], R)
TESTS[0x40067678] = mcase(lambda r: [r.choice([0, 1, 100, 0xFFFFFFFF, r.getrandbits(32)]), r.choice([0, 1, 8, r.getrandbits(32)])], R)
TESTS[0x400676BC] = mcase(lambda r: [mn(r), r.choice([0, 1, 5, 50]), r.choice([4, 8, 0x4004, r.getrandbits(32)])], R)
for a in (0x4006773C, 0x400678B0, 0x400678D8, 0x40067918, 0x40067958, 0x40067A04, 0x40067D1C):
    TESTS[a] = mcase(lambda r: [mn(r)], R)
TESTS[0x400677A8] = mcase(lambda r: [], R)
HZ = lambda r: r.choice([1, 400000, 400001, 1000000, 24000000, 24000001, 25000000, 25000001, 50000000, 50000001,
                         100000000, r.randrange(1, 200000000)])
TESTS[0x400677E4] = mcase(lambda r: [mn(r), HZ(r)], R)
TESTS[0x4006797C] = mcase(lambda r: [mn(r), HZ(r)], R)
TESTS[0x40067A30] = mcase(lambda r: [mn(r), BUF + 0x3000], R)
TESTS[0x40067B30] = mcase(lambda r: [mn(r), BUF + 0x3000, BUF + 0x1000], R, big=True)   # up to 257 descriptors
TESTS[0x40067CFC] = mcase(lambda r: [mn(r), r.getrandbits(32)], R)
TESTS[0x400688BC] = mcase(lambda r: [BUF + r.getrandbits(12)])
TESTS[0x400688C8] = mcase(lambda r: (lambda s: [s, s + r.choice([0, 1, 31, 32, 100, 4096])])(BUF + r.getrandbits(12)))
TESTS[0x400688FC] = mcase(lambda r: [BUF + r.getrandbits(12), r.choice([0, 1, 31, 32, 100, 4096])])
