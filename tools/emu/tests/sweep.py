"""Behavioural tests for batch 28 onwards: small leftovers (setters, gates, one-byte FPGA commands,
drawing wrappers, fixed call sequences).

This suite is kept apart from fpga.py so that adding tests here does not invalidate the cached
results of the older suites. The helpers are copied, not imported, because the cache key of a
suite covers only its own file.
"""
from unicorn import arm_const as A

from tools.emu.emu import Stub
from tools.emu.tests.fatfs import Case

N = 200
V, R = 0, 32
SYS = 0x40544E68
TOUCH_BLK = 0x42E68BB0
SEL, WR, RD = 0x4004927C, 0x40049214, 0x400491B8

TESTS = {}


def rec(name, addr, nargs, ret=None):
    return Stub(name, addr, (lambda m, a: ret if not callable(ret) else ret(m, a)), nargs)


def rec_stk(name, addr, nstack):
    """Records r0..r3 and the first nstack stack words (plain stubs see only r0..r3)."""
    def fn(m, a):
        sp = m.reg(A.UC_ARM_REG_SP)
        m.calls.append((name + "/stack",) + tuple(m.r32(sp + 4 * i) for i in range(nstack)))
    return Stub(name, addr, fn, 4)


def regs(m, n):
    return [m.reg(x) for x in (A.UC_ARM_REG_R0, A.UC_ARM_REG_R1, A.UC_ARM_REG_R2, A.UC_ARM_REG_R3)[:n]]


def quiet(name, addr, fn, nargs):
    """Stub that sees nargs argument registers but records nothing by itself (for stack pointers
    such as a FIL on the caller's frame, whose address depends on the frame layout)."""
    return Stub(name, addr, lambda m, a: fn(m, regs(m, nargs)), 0)


def rand_block(r, base, n):
    data = bytes(r.getrandbits(8) for _ in range(n))
    return lambda m: m.write(base, data)


def compose(*fns):
    return lambda m: [f(m) for f in fns]


def sys_fields(r, fields):
    """{offset: (size, choices)}: each field gets a random pick from its choices (or random bits)."""
    vals = {}
    for o, (sz, ch) in fields.items():
        vals[o] = (sz, r.choice(ch) if ch else r.getrandbits(8 * sz))

    def setup(m):
        for o, (sz, v) in vals.items():
            m.write(SYS + o, (v & ((1 << (8 * sz)) - 1)).to_bytes(sz, "little"))
    return setup


def bus(reads=()):
    """FPGA bus stubs; reads come from m.rd, which the setup fills (setup runs once per version)."""
    return (rec("sel", SEL, 1), rec("wr", WR, 1), Stub("rd", RD, lambda m, a: m.rd.pop(0), 0))


def with_reads(vals):
    def setup(m):
        m.rd = list(vals)
    return setup


def t17(r):
    v = r.choice([0, 0, 1, 0x80, r.getrandbits(8)])
    return lambda m: m.w8(TOUCH_BLK + 0x17, v)


def b652(r):
    v = r.choice([0, 0, 1, r.getrandbits(8)])
    return lambda m: m.w8(SYS + 0x652, v)


def rnd(r):
    return r.choice([0, 1, 0xFF, 0x100, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, r.getrandbits(32),
                     r.getrandbits(32), r.getrandbits(16)])


# ---------------------------------------------------------------- setters
TESTS[0x4002CCCC] = lambda r: Case([rnd(r)], rand_block(r, 0x42E68C70, 12), ret=V)
TESTS[0x4002DC40] = lambda r: Case([rnd(r)], rand_block(r, 0x42E68BE0, 8), ret=V)
TESTS[0x4002DC74] = lambda r: Case([rnd(r)], rand_block(r, 0x42E68BE0, 8), ret=V)
TESTS[0x400342CC] = lambda r: Case([], rand_block(r, 0x40095494, 12), ret=V)

# ---------------------------------------------------------------- gates
for _a, _t in ((0x40023E90, 0x40023EA4), (0x40024C4C, 0x40024C60), (0x40028350, 0x40028364),
               (0x40037C28, 0x40037C3C)):
    TESTS[_a] = (lambda t: lambda r: Case([], t17(r), ret=V, stubs=(rec("t", t, 0),)))(_t)
TESTS[0x40024E14] = lambda r: Case([], compose(t17(r), b652(r)), ret=V,
                                   stubs=(rec("a", 0x40023EA4, 0), rec("b", 0x40024C60, 0)))
TESTS[0x4002CC54] = lambda r: Case([], b652(r), ret=V, stubs=(rec("a", 0x4002C7AC, 0), rec("b", 0x4002C5F8, 0)))
TESTS[0x4002CC6C] = lambda r: Case([], compose(t17(r), b652(r)), ret=V,
                                   stubs=(rec("e10c", 0x4003C550, 0, 1), rec("fill", 0x4002C5C4, 0),
                                          rec("a", 0x4002C7AC, 0), rec("b", 0x4002C5F8, 0)))


# ---------------------------------------------------------------- drawing wrappers
def t_pixel(r):
    x, y = r.randrange(0, 1100), r.randrange(0, 650)
    tgt = 0x41000000 + 2 * r.randrange(0, 0x100)
    flag = r.choice([0, 0, 1, r.getrandbits(8)])
    col = r.getrandbits(16)

    def setup(m):
        m.w8(0x42E6865C, flag)
        m.write(0x42E68660, col.to_bytes(2, "little"))
        m.write(0x40095498, tgt.to_bytes(4, "little"))
    return Case([x, y], setup, ret=V, stubs=(rec("blend", 0x4002EA88, 2),))


TESTS[0x4002EB70] = t_pixel
TESTS[0x4002EBA8] = lambda r: Case([rnd(r), rnd(r), rnd(r)], ret=V, stubs=(rec("ecc4", 0x4002ECC4, 4),))
TESTS[0x4002C5C4] = lambda r: Case([], rand_block(r, SYS, 0x10), ret=V,
                                   stubs=(rec("col", 0x4002EA38, 1), rec("ecc4", 0x4002ECC4, 4)))
TESTS[0x40034284] = lambda r: Case([], rand_block(r, 0x42E68C38, 8), ret=V, stubs=(rec("436c", 0x4003436C, 4),))
TESTS[0x40031710] = lambda r: Case([], (lambda v: lambda m: m.w8(0x42E68D1A, v))(r.choice([0, 1, r.getrandbits(8)])),
                                   ret=V, stubs=(rec("col", 0x4002EA38, 1),))
TESTS[0x4001C348] = lambda r: Case([], rand_block(r, 0x42E64560, 0x20), ret=V,
                                   stubs=(rec_stk("rect", 0x4002F160, 1), rec("46c0", 0x400346C0, 0)))
TESTS[0x4002DC4C] = lambda r: Case([], rand_block(r, 0x42E68BE0, 8), ret=V,
                                   stubs=(rec("scatter", 0x4003CA38, 0), rec("ccd8", 0x4002CCD8, 0)))
TESTS[0x400344E0] = lambda r: Case([], rand_block(r, 0x42E68C70, 12), ret=V,
                                   stubs=(rec("defaults", 0x4002DC4C, 0), rec("col", 0x4002EA38, 1),
                                          rec("lcd", 0x4003E688, 0), rec("clear", 0x4003446C, 0)))


# ---------------------------------------------------------------- one-byte FPGA commands
def t_fpga(fields, reads=0, extra=()):
    def t(r):
        vals = [r.getrandbits(8) for _ in range(reads)]
        return Case([], compose(sys_fields(r, fields), with_reads(vals)), ret=V, stubs=bus() + tuple(extra))
    return t


ANY = (1, None)
FLAG = (1, [0, 0, 1, 2, 0x80, 0xFF])
TESTS[0x400635D0] = t_fpga({0x3B: ANY})
TESTS[0x40063460] = t_fpga({0x27B: ANY})
TESTS[0x4004EEEC] = lambda r: Case([], rand_block(r, 0x42E428E4, 4), ret=V, stubs=bus())
TESTS[0x40063430] = t_fpga({0x27C: FLAG})
TESTS[0x40063354] = t_fpga({0x276: (2, None)})
TESTS[0x4006366C] = t_fpga({0x274: FLAG, 0x275: FLAG, 0x27A: FLAG})
TESTS[0x400636BC] = t_fpga({0x274: FLAG, 0x275: FLAG, 0x288: (4, None)})
TESTS[0x400635F4] = t_fpga({0x3B: ANY}, extra=(rec("c61", 0x400635D0, 0),))
TESTS[0x40063628] = t_fpga({0x274: FLAG, 0x275: FLAG, 0x280: (2, None)}, extra=(rec("pulse", 0x400635F4, 0),))
TESTS[0x40063770] = t_fpga({0x279: ANY}, reads=1)
TESTS[0x40063328] = lambda r: Case([], ret=V, stubs=tuple(rec(n, a, 0) for n, a in (
    ("c61", 0x400635D0), ("c0c", 0x40063354), ("c1a", 0x4006366C), ("c15", 0x40063430), ("c16", 0x40063460),
    ("34f0", 0x400634F0), ("trig", 0x4003FAE8))))
TESTS[0x4004F59C] = lambda r: Case([], compose(rand_block(r, SYS + 0x7F8, 8), rand_block(r, 0x42E68AE0, 0x20)), ret=V,
                                   stubs=(rec("probe", 0x400492E4, 0),))
TESTS[0x4004F5C0] = lambda r: Case([], compose(rand_block(r, 0x42E62C2C + 0x8F4, 8), rand_block(r, 0x42E68AE0, 0x20)),
                                   ret=V, stubs=(rec("sec", 0x40049E00, 0),))


def t_pair(r):
    a, b = r.choice([0, 1, 2, 3, r.getrandbits(32)]), r.choice([0, 1, 3, r.getrandbits(32)])
    return Case([], rand_block(r, 0x42E68B20, 8), ret=V,
                stubs=(rec("3a50", 0x40063A50, 0), rec("4c28", 0x40064C28, 0, a), rec("4cf0", 0x40064CF0, 0, b),
                       rec("save", 0x400439D8, 0), rec("eb40", 0x4003EB40, 0)))


TESTS[0x40064DC0] = t_pair

# ---------------------------------------------------------------- loose wrappers
TESTS[0x400525D4] = lambda r: Case([], ret=V, stubs=(rec("btn", 0x4002F408, 0), rec("45f8", 0x400545F8, 0)))
TESTS[0x40063CB0] = lambda r: Case([], ret=V, stubs=(rec("3c34", 0x40063C34, 1),))
for _a in (0x4002D6D4, 0x4002D6F8):
    TESTS[_a] = lambda r: Case([rnd(r), rnd(r), rnd(r), rnd(r)], ret=V, stack=[rnd(r)],
                               stubs=(rec_stk("d82c", 0x4002D82C, 2),))
TESTS[0x4002D71C] = lambda r: Case([rnd(r), rnd(r), rnd(r), rnd(r)], ret=V, stubs=(rec("d2d4", 0x4002D2D4, 4),))
TESTS[0x40034058] = lambda r: Case([], ret=V, stubs=(rec("2d3c", 0x40032D3C, 0), rec("2e0c", 0x40032E0C, 1)))


def t_table(r):
    tbl = [r.getrandbits(16) for _ in range(24)]
    v = r.choice([r.getrandbits(32), tbl[r.randrange(24)], -((tbl[r.randrange(24)] ^ 0x8000) - 0x8000),
                  (tbl[r.randrange(24)] ^ 0x8000) - 0x8000, 0])
    mode = r.choice([0, 1, 1, 2, r.getrandbits(32)])
    return Case([mode, v & 0xFFFFFFFF],
                lambda m: m.write(0x405456E8, b"".join(t.to_bytes(2, "little") for t in tbl)), ret=R)


TESTS[0x400301A8] = t_table
TESTS[0x400367A4] = lambda r: Case([], compose(rand_block(r, SYS, 0x700), rand_block(r, SYS + 0x3DA528, 8)), ret=V)
TESTS[0x400367D0] = lambda r: Case([], compose(rand_block(r, SYS, 0x40), rand_block(r, SYS + 0x3DA528, 8)), ret=V)
TESTS[0x400373E0] = lambda r: Case([], rand_block(r, 0x40545E00, 8), ret=V, stubs=tuple(
    rec(n, a, 1 if n == "6d88" else 0) for n, a in (
        ("6b00", 0x40036B00), ("6bdc", 0x40036BDC), ("709c", 0x4003709C), ("6864", 0x40036864), ("6de4", 0x40036DE4),
        ("6d88", 0x40036D88), ("6c54", 0x40036C54), ("6d10", 0x40036D10))))
TESTS[0x400378E0] = lambda r: Case([], rand_block(r, SYS + 0xF88, 0x28), ret=V, stubs=tuple(rec(n, a, 0) for n, a in (
    ("6b00", 0x40036B00), ("6bdc", 0x40036BDC), ("77f8", 0x400377F8), ("771c", 0x4003771C))))
TESTS[0x40035744] = lambda r: Case([], rand_block(r, SYS + 0xF88, 0x28), ret=V, stubs=tuple(rec(n, a, 0) for n, a in (
    ("58b8", 0x400358B8), ("63cc", 0x400363CC), ("62f4", 0x400362F4), ("656c", 0x4003656C), ("65f4", 0x400365F4))))


# ================================================================ batch 29
def s16(v):
    return (v & 0xFFFF).to_bytes(2, "little")


def i32(v):
    return (v & 0xFFFFFFFF).to_bytes(4, "little")


def writes(items):
    """[(address, bytes)] written in order."""
    return lambda m: [m.write(a, b) for a, b in items]


TESTS[0x4002EA38] = lambda r: Case([rnd(r)], rand_block(r, 0x42E6865C, 16), ret=V)


def t_rect_copy(r):
    x0 = r.randrange(0, 1000)
    x1 = r.choice([x0, x0 + r.randrange(1, 40), max(0, x0 - 3)])
    y0 = r.randrange(0, 600)
    y1 = r.choice([y0, y0 + r.randrange(1, 6), max(0, y0 - 2)])
    rows = []
    xa, xb = max(0, min(x0, x1) - 1), max(x0, x1) + 1
    for y in range(max(0, min(y0, y1) - 1), max(y0, y1) + 2):
        rows.append((0x420E4E84 + 2 * (xa + 1022 * y), bytes(r.getrandbits(8) for _ in range(2 * (xb - xa)))))
    return Case([x0, y0, x1, y1], writes(rows), ret=V, stubs=(rec("dc", 0x4003E6F4, 4),))


TESTS[0x4003436C] = t_rect_copy

SMALLN = [0, 0, 1, 2, 3, 7, 15, 20]


def t_insert(cnt):
    def t(r):
        n = r.choice(SMALLN)
        c = r.choice([-1, 0, 1, 5, 100, r.getrandbits(16)])
        blk = bytes(r.getrandbits(8) for _ in range(0x40))
        return Case([rnd(r)], writes([(SYS + 0xFA0, blk), (SYS + cnt, s16(n)), (SYS + 0xFA0, s16(c))]), ret=V)
    return t


for _a, _c in ((0x400349B4, 0x754), (0x40035A00, 0x756), (0x40036D88, 0x758)):
    TESTS[_a] = t_insert(_c)


def t_count(r):
    n = r.choice([-1, -0x8000, 0, 1, 2, 17, 40])
    data = bytes(r.choice([0, 0, 1, r.getrandbits(8)]) for _ in range(48))
    return Case([], writes([(SYS + 0x88BC, bytes(r.getrandbits(8) for _ in range(4))), (SYS + 0x5DC6, data),
                            (SYS + 0xFA0, s16(n))]), ret=V)


for _a in (0x400353A8, 0x40036454, 0x40037880):
    TESTS[_a] = t_count

CV = [-0x8000, -3, -1, 0, 1, 2, 5, 30, 31, 100, 0x7FFF]


def t_clamp(r):
    vals = [r.choice(CV + [r.getrandbits(16)]) for _ in range(5)]
    return Case([], writes([(SYS + 0xF9E + 2 * i, s16(v)) for i, v in enumerate(vals)]), ret=V)


for _a in (0x40035248, 0x400362F4, 0x4003771C):
    TESTS[_a] = t_clamp


def t_advance(r):
    i = r.choice([0, 1, 2, 5, 9, -1, r.getrandbits(32)]) if r.random() < 0.9 else r.randrange(0, 12)
    ii = i if i < 0x80000000 else i - (1 << 32)
    blk = bytearray(r.getrandbits(8) for _ in range(0x40))
    cur = r.getrandbits(16)
    items = [(SYS + 0xFA0, bytes(blk))]
    if -1 <= ii < 12:
        cnt = r.choice([ii, ii + 1, ii + 2, ii + 5, 0])
        items.append((SYS + 0xFA0, s16(cnt)))
        if r.random() < 0.7:
            items.append((SYS + 0xFA6 + 2 * ii, s16(cur)))
        stray = []
    else:
        ii = 0
        i = 0
    items.append((SYS + 0x3DA52C, bytes(r.getrandbits(8) for _ in range(4))))
    items.append((SYS + 0x3DA52E, s16(cur)))
    return Case([i & 0xFFFFFFFF], writes(items), ret=V)


TESTS[0x40037418] = t_advance


def t_decimate(r):
    n = r.choice([-1, 0, 1, 2, 3, 5, 0x80000000])
    w = r.choice([0, 1, 7, 40, 100, -5])
    data = bytes(r.getrandbits(8) for _ in range(220))
    return Case([n & 0xFFFFFFFF], writes([(SYS + 0x3DA53C, i32(w)), (SYS + 0x3DA548, data)]), ret=V)


TESTS[0x400374EC] = t_decimate
WV = [-5, 0, 11, 12, 13, 500, 588, 1000, 0x7FFFFFFF, -0x80000000]
HV = [0, 10, 24, 27, 600, 1022, 0xFFFF]


def t_cursor(a0, h):
    def t(r):
        v0, v1 = r.choice(WV + [r.getrandbits(32)]), r.choice(WV + [r.getrandbits(32)])
        hv = r.choice(HV + [r.getrandbits(16)])
        return Case([], writes([(SYS + a0, i32(v0)), (SYS + a0 + 4, i32(v1)), (SYS + h, s16(hv))]), ret=V)
    return t


TESTS[0x400340D4] = t_cursor(0x810, 0xA)
TESTS[0x4003407C] = t_cursor(0x808, 8)


def t_ac64(r):
    b = 0x42E180C8
    v0, v1 = r.choice([-1, 0, 1, 2, 3, 500, 1000, r.getrandbits(32)]), r.choice([-1, 0, 1, 2, 3, 500, r.getrandbits(32)])
    lim = r.choice([0, 1, 2, 3, 4, 300, 1022, r.getrandbits(16)])
    return Case([], writes([(b + 4, s16(lim)), (b + 0xAC64, i32(v0)), (b + 0xAC68, i32(v1))]), ret=V)


TESTS[0x40037A60] = t_ac64


def rec_d0(name, addr, nargs):
    def fn(m, a):
        m.calls.append((name + "/d0", m.reg(A.UC_ARM_REG_D0)))
    return Stub(name, addr, fn, nargs)


def t_label(r):
    c = Case([rnd(r), rnd(r), rnd(r)], rand_block(r, 0x42E68BE0, 8), ret=V,
             stubs=(rec_d0("c00", 0x40038C00, 3), rec("ce7c", 0x4002CE7C, 3)))
    c.dregs = {0: r.getrandbits(64)}
    return c


TESTS[0x40038DAC] = t_label
TESTS[0x40038DEC] = t_label


def t_exists(build):
    def t(r):
        rc = r.choice([0, 0, 1, 4, 5])

        def op(m, a):
            m.fil = a[0]
            m.calls.append(("open", a[1], a[2] & 0xFF))
            return rc

        def cl(m, a):
            m.calls.append(("close", a[0] == m.fil))
            return 0
        return Case([rnd(r)], ret=R, stubs=(rec("path", build, 1), quiet("f_open", 0x40059B8C, op, 3),
                                            quiet("f_close", 0x4005A694, cl, 1)))
    return t


TESTS[0x400353F4] = t_exists(0x40034B78)
TESTS[0x400364A0] = t_exists(0x40035BC4)
TESTS[0x400346C0] = lambda r: Case([], ret=V, stubs=tuple(rec("%08x" % a, a, 0) for a in (
    0x4003E708, 0x4002CC6C, 0x400419CC, 0x4003B7B8, 0x4003C970, 0x40041F5C, 0x40042520, 0x40024E14, 0x4003117C,
    0x40030F8C, 0x40031794, 0x40028350)))


def t_win40(r):
    blk = bytes(r.choice([0, 0, 1, r.getrandbits(8)]) for _ in range(8))
    return Case([], writes([(0x42E68668, blk)]), ret=V, stubs=tuple(rec("%08x" % a, a, 0) for a in (
        0x400612FC, 0x40060FE8, 0x40060DC8, 0x40061F10, 0x400617AC, 0x400623F4, 0x40062448, 0x40062B1C, 0x40062A18,
        0x40062C98, 0x40061194, 0x4006068C)))


TESTS[0x40062EE8] = t_win40
TESTS[0x40063714] = t_fpga({0x274: FLAG, 0x275: FLAG, 0x28C: (4, None)})


# ================================================================ batch 30: file lists A, B, C
F_OPEN, F_CLOSE, F_READ, F_WRITE, F_LSEEK, F_UNLINK = (0x40059B8C, 0x4005A694, 0x40059F7C, 0x4005A1D4, 0x4005A6CC,
                                                       0x4005AD8C)


def fs_stubs():
    """FatFs stubs. f_open answers from m.rcs (default 0) and remembers the FIL pointer; the
    other calls record whether they got that same pointer. f_read fills the buffer from m.rdata."""
    def op(m, a):
        m.fil = a[0]
        m.calls.append(("open", a[1], a[2] & 0xFF))
        return m.rcs.pop(0) if m.rcs else 0

    def same(name, extra):
        def fn(m, a):
            m.calls.append((name, a[0] == getattr(m, "fil", None)) + tuple(extra(m, a)))
            return 0
        return fn

    def wr(m, a):
        m.calls.append(("write", a[0] == getattr(m, "fil", None), a[1], a[2], a[3]))
        return getattr(m, "wrc", 0)

    def rd(m, a):
        n = min(a[2], len(m.rdata))
        m.write(a[1], m.rdata[:n])
        m.calls.append(("read", a[0] == m.fil, a[1], a[2], a[3]))
        return 0
    return (quiet("f_open", F_OPEN, op, 3), quiet("f_close", F_CLOSE, same("close", lambda m, a: ()), 1),
            quiet("f_lseek", F_LSEEK, same("lseek", lambda m, a: (a[2], a[3])), 4),
            quiet("f_write", F_WRITE, wr, 4),
            quiet("f_read", F_READ, rd, 4), rec("unlink", F_UNLINK, 1))


def fs_setup(r, nopen=1, p_fail=0.3):
    rcs = [r.choice([1, 4, 5]) if r.random() < p_fail else 0 for _ in range(nopen)]
    rdata = bytes(r.getrandbits(8) for _ in range(0x60))

    def setup(m):
        m.rcs = list(rcs)
        m.rdata = rdata
    return setup


PS = [1, 2, 3, 5, 10, 16]
CNT = [-1, 0, 1, 2, 7, 12, 30]


def list_area(r, ps=None, cnt=None, sel=None, page=None):
    items = [(SYS + 0xF90, bytes(r.getrandbits(8) for _ in range(0x60))),
             (SYS + 0x5DC6, bytes(r.choice([0, 0, 1, r.getrandbits(8)]) for _ in range(0x48))),
             (SYS + 0x84D0, bytes(r.getrandbits(8) for _ in range(0x10)))]
    for o, v in ((0xF9A, ps), (0xFA0, cnt), (0xFA4, sel), (0xF9E, page)):
        if v is not None:
            items.append((SYS + o, s16(v)))
    return items


def t_page_mark(r):
    return Case([], writes(list_area(r, r.choice(PS), r.choice(CNT), page=r.choice([0, 0, 1, 2, 3, -1]))), ret=V,
                stubs=(rec("memset", 0x4005C604, 3),))


for _a in (0x400352A8, 0x40036354, 0x4003777C):
    TESTS[_a] = t_page_mark


def t_marks_clear(r):
    return Case([], writes(list_area(r, r.choice(PS), r.choice(CNT))), ret=V)


for _a in (0x40035320, 0x400363CC, 0x400377F8):
    TESTS[_a] = t_marks_clear


def t_index_load(fail, cnt):
    def t(r):
        n = r.choice([0, 1, 5, 20, r.getrandbits(16)])
        return Case([], compose(writes(list_area(r, r.choice(PS)) + [(SYS + cnt, s16(n))]), fs_setup(r)), ret=V,
                    stubs=fs_stubs() + (rec("fail", fail, 0),))
    return t


def t_index_save(fail, cnt):
    def t(r):
        n = r.choice([0, 1, 5, 20, r.getrandbits(16)])
        return Case([], compose(writes([(SYS + cnt, s16(n))]), fs_setup(r)), ret=V,
                    stubs=fs_stubs() + (rec("fail", fail, 0),))
    return t


def t_prune(path, remove, tail):
    def t(r):
        return Case([], compose(writes(list_area(r, r.choice(PS), r.choice(CNT[:5]))), fs_setup(r, 8, 0.4)), ret=V,
                    stubs=fs_stubs() + (rec("path", path, 1), rec("remove", remove, 1)) +
                    tuple(rec("%08x" % x, x, 0) for x in tail))
    return t


def t_validate(exists, prune, clamp):
    def t(r):
        ex = [r.choice([1, 1, 1, 0, 2]) for _ in range(20)]
        ps = r.choice([1, 2, 4, 8])
        sel = r.choice([0, 1, 3, 5, 9, -1])

        def setup(m):
            m.ex = list(ex)
        ents = b"".join(s16(r.choice([0, 1, 7, r.getrandbits(16), r.getrandbits(16)])) for _ in range(80))
        area = list_area(r, ps, sel=sel)
        return Case([], compose(writes(area[:1] + [(SYS + 0xFA6, ents)] + area[1:]), setup), ret=V,
                    stubs=(Stub("exists", exists, lambda m, a: (m.calls.append(("exists", a[0])), m.ex.pop(0))[1], 1),
                           rec("prune", prune, 0), rec("clamp", clamp, 0)))
    return t


def t_delete(path, remove, tail):
    def t(r):
        return Case([], writes(list_area(r, r.choice(PS), r.choice(CNT[:5]))), ret=V,
                    stubs=(rec("path", path, 1), rec("remove", remove, 1), rec("unlink", F_UNLINK, 1)) +
                    tuple(rec("%08x" % x, x, 0) for x in tail))
    return t


TESTS[0x4003486C] = t_index_load(0x400347A0, 0x754)
TESTS[0x400358B8] = t_index_load(0x400357EC, 0x756)
TESTS[0x40034920] = t_index_save(0x400347A0, 0x754)
TESTS[0x4003596C] = t_index_save(0x400357EC, 0x756)
TESTS[0x40036C54] = t_index_save(0x40036958, 0x758)
TESTS[0x40035434] = t_prune(0x40034B78, 0x40034A0C, (0x40034920, 0x4003486C, 0x40035248))
TESTS[0x400364E0] = t_prune(0x40035BC4, 0x40035A58, (0x4003596C, 0x400358B8, 0x400362F4))
TESTS[0x400354C0] = t_validate(0x400353F4, 0x40035434, 0x40035248)
TESTS[0x4003656C] = t_validate(0x400364A0, 0x400364E0, 0x400362F4)
TESTS[0x400350F4] = t_delete(0x40034B78, 0x40034A0C, (0x40035320, 0x40034920, 0x4003486C))
TESTS[0x400361CC] = t_delete(0x40035BC4, 0x40035A58, (0x400363CC, 0x4003596C, 0x400358B8))


def t_data(r):
    w_, h = r.choice([0, 1, 0x20, 0x1000, r.getrandbits(32)]), r.choice([0, 1, 3, r.getrandbits(16)])
    return Case([], compose(writes([(SYS + 0x3DA544, i32(w_)), (SYS + 0x758, s16(h))]), fs_setup(r)), ret=V,
                stubs=fs_stubs() + (rec("fail", 0x40036958, 0),))


TESTS[0x40036BDC] = t_data
TESTS[0x40036D10] = t_data


# ================================================================ batch 31: file lists, second part
F_MKDIR, F_CHMOD, FILL_U16, COPY_U16 = 0x4005AED4, 0x4005B130, 0x400356F4, 0x40035728
LCNT = [0, 1, 2, 3, 5, 12, 20, 40]


def ents(r, n=80, top=None):
    """n u16 entries: zeros, small numbers (up to top) and random values."""
    top = top or 50
    return b"".join(s16(r.choice([0, 1, 2, r.randrange(top), r.randrange(top), r.randrange(top),
                                  r.getrandbits(16)])) for _ in range(n))


def t_remove(cnt):
    def t(r):
        e = ents(r)
        n = r.choice(LCNT)
        v = r.choice([int.from_bytes(e[2 * i:2 * i + 2], "little") for i in range(max(n, 1))] +
                     [0, 0x10000 + r.getrandbits(8), r.getrandbits(16)])
        return Case([v], writes([(SYS + 0xF90, bytes(r.getrandbits(8) for _ in range(0x16))), (SYS + 0xFA6, e),
                                 (SYS + cnt, s16(n)), (SYS + 0xFA0, s16(r.choice(CNT + [0x7FFF])))]), ret=V)
    return t


def t_next_free(cnt):
    def t(r):
        n = r.choice(LCNT)
        e = ents(r, 60, top=r.choice([5, 20, 45]))
        return Case([], writes([(SYS + 0xF90, bytes(r.getrandbits(8) for _ in range(0x16))), (SYS + 0xFA6, e),
                                (SYS + 0x8924, bytes(r.getrandbits(8) for _ in range(0x80))),
                                (SYS + cnt, s16(n)), (SYS + 0xFA0, s16(r.choice(CNT + [40, 60])))]), ret=V)
    return t


for _l, _c in ((0x40034A0C, 0x754), (0x40035A58, 0x756), (0x40036F74, 0x758)):
    TESTS[_l] = t_remove(_c)
for _l, _c in ((0x40034AB4, 0x754), (0x40035B00, 0x756), (0x4003709C, 0x758)):
    TESTS[_l] = t_next_free(_c)


def mk_stubs():
    return fs_stubs() + (rec("mkdir", F_MKDIR, 1), rec("chmod", F_CHMOD, 3), rec("fill", FILL_U16, 3),
                         rec("copy", COPY_U16, 3))


def t_create(cnt):
    def t(r):
        return Case([], compose(writes([(SYS + 0xF90, bytes(r.getrandbits(8) for _ in range(0x16))),
                                        (SYS + cnt, s16(r.choice(LCNT + [r.getrandbits(16)]))),
                                        (SYS + 0x3DA52E, s16(r.getrandbits(16))),
                                        (SYS + 0x3DA544, i32(r.choice([0, 1, 7, r.getrandbits(32)])))]),
                                fs_setup(r, 2)), ret=V, stubs=mk_stubs())
    return t


TESTS[0x400347A0] = t_create(0x754)
TESTS[0x400357EC] = t_create(0x756)
TESTS[0x40036958] = t_create(0x758)
TESTS[0x40036B00] = t_index_load(0x40036958, 0x758)


def t_write_file(r):
    wrc = r.choice([0, 0, 1, 7])
    st = fs_setup(r)

    def setup(m):
        st(m)
        m.wrc = wrc
    return Case([rnd(r)], compose(writes([(SYS + 0x774, i32(rnd(r)))]), setup), ret=R, stubs=fs_stubs())


TESTS[0x40035780] = t_write_file


def t_thumb(r):
    return Case([], compose(writes([(SYS + 0x109680, bytes(r.getrandbits(8) for _ in range(0x10)))]), fs_setup(r)),
                ret=V, stubs=fs_stubs())


TESTS[0x40035188] = t_thumb


def t_bfile(r):
    return Case([], compose(writes([(SYS + 0x760, i32(r.choice([0, 0x1C, 0x40, 0x1000]))),
                                    (SYS + 0x76C, i32(rnd(r)))]), fs_setup(r)),
                ret=V, stubs=fs_stubs() + (rec("h", 0x40045670, 0), rec("i", 0x400456AC, 0)))


TESTS[0x40036260] = t_bfile


def t_thumbs(path, load):
    def t(r):
        whs = [r.getrandbits(32) for _ in range(16)]

        def ld(m, a):
            m.calls.append(("load",))
            m.w32(SYS + 0x109684, m.whs.pop(0))

        def setup(m):
            m.whs = list(whs)
        e = ents(r, 80)
        return Case([], compose(writes(list_area(r, r.choice(PS), page=r.choice([0, 0, 1, 2, 3, -1])) +
                                       [(SYS + 0xFA6, e), (SYS + 0x109684, i32(r.getrandbits(32)))]), setup),
                    ret=V, stubs=(rec("path", path, 1), Stub("load", load, ld, 0), rec("copy", COPY_U16, 3)))
    return t


TESTS[0x40035578] = t_thumbs(0x40034B78, 0x40035188)
TESTS[0x400365F4] = t_thumbs(0x40035BC4, 0x40036260)


def t_cur_a(r):
    return Case([], compose(writes(list_area(r, sel=r.choice([0, 1, 5, 30, -1, -2])) + [(SYS + 0xFA6, ents(r))]),
                            fs_setup(r)), ret=V, stubs=fs_stubs() + (rec("path", 0x40034B78, 1),))


def t_cur_b(r):
    return Case([], compose(writes(list_area(r, sel=r.choice([0, 1, 5, 30, -1, -2])) +
                                   [(SYS + 0xFA6, ents(r)), (SYS + 0x774, i32(rnd(r))),
                                    (SYS + 0x3C, bytes([r.getrandbits(8)])),
                                    (SYS + 0x665, bytes([r.choice([0, 0, 1, r.getrandbits(8)])]))]),
                            fs_setup(r)), ret=V,
                stubs=fs_stubs() + (rec("path", 0x40035BC4, 1), rec("restore", 0x4003F0E0, 0),
                                    rec("backup", 0x4003F0C0, 0), rec("h", 0x40045670, 0),
                                    rec("img", 0x400456F4, 0)))


TESTS[0x40035624] = t_cur_a
TESTS[0x400366A0] = t_cur_b

TBL = 0x42C2FE74


def nf_stub(addr):
    """next-free stub: writes the chosen number m.nf to SYS+0xF9C."""
    def fn(m, a):
        m.calls.append(("next_free",))
        m.write(SYS + 0xF9C, s16(m.nf))
    return Stub("next_free", addr, fn, 0)


def t_save_a(r):
    rc, nf = r.choice([0, 0, 1, 2, 0xFFFFFFFF]), r.getrandbits(16)

    def setup(m):
        m.nf = nf
    return Case([], compose(rand_block(r, SYS, 0x10), rand_block(r, TBL + 0xCB14, 0x10),
                            writes([(SYS + 0xF9C, s16(r.getrandbits(16)))]), setup), ret=V,
                stubs=(rec("4e94", 0x40034E94, 0), rec("img", 0x40034134, 4), rec("load", 0x4003486C, 0),
                       nf_stub(0x40034AB4), rec("path", 0x40034B78, 1), rec("bmp", 0x4005C3F0, 1, ret=rc),
                       rec("insert", 0x400349B4, 1), rec("save", 0x40034920, 0), rec("rect", 0x40034284, 0)))


def t_save_b(r):
    s774 = r.choice([0, 1, 0x1000, 0x7FFFFFFF, 0x80000000, r.getrandbits(32)])
    free = r.choice([0, (2 * s774) & 0xFFFFFFFF, (2 * s774 - 1) & 0xFFFFFFFF, (2 * s774 + 1) & 0xFFFFFFFF,
                     0xFFFFFFFF, r.getrandbits(32)])
    rc, nf = r.choice([0, 0, 1, 2]), r.getrandbits(16)

    def setup(m):
        m.nf = nf
    return Case([], compose(rand_block(r, SYS, 0x10), rand_block(r, TBL + 0xC980, 0x198),
                            rand_block(r, TBL + 0xD820, 4), rand_block(r, TBL + 0xD9B4, 2),
                            writes([(SYS + 0xF9C, s16(r.getrandbits(16))), (SYS + 0x774, i32(s774))]), setup),
                ret=V,
                stubs=(rec("free", 0x40049BE4, 0, ret=free), rec("delay", 0x40042CF0, 1), rec("load", 0x400358B8, 0),
                       nf_stub(0x40035B00), rec("5ee0", 0x40035EE0, 0), rec("img", 0x40034134, 4),
                       rec("path", 0x40035BC4, 1), rec("enc", 0x40045714, 0), rec("wf", 0x40035780, 1, ret=rc),
                       rec("insert", 0x40035A00, 1), rec("save", 0x4003596C, 0), rec("rect", 0x40034284, 0)))


TESTS[0x4003506C] = t_save_a
TESTS[0x400360F0] = t_save_b


def t_record(r):
    st = r.choice([1, 2, 4, 5, 8, 0, 0xFFFFFFFF])
    n = r.choice([0, 1, 3, 6])
    blk = bytes(r.choice([0, 1, 2, r.getrandbits(8)]) for _ in range(0x120))
    hs = [int.from_bytes(blk[2 * i:2 * i + 2], "little") for i in range(0x90)]
    keys = [((hs[i] & 0xFF) << 8) | (hs[i + 1] & 0xFF) for i in range(0, 0x40, max(1, st if st < 9 else 1))]
    v = r.choice(keys + [r.getrandbits(16), 0x10000 | keys[0]])
    return Case([v], writes([(SYS + 0xD744, blk), (SYS + 0x758, s16(n)), (SYS + 0x3DA544, i32(st)),
                             (SYS + 0x3DA53C, i32(r.getrandbits(32))), (SYS + 0xF9C, s16(r.getrandbits(16)))]),
                ret=V, stubs=(rec("copy", COPY_U16, 3),))


TESTS[0x40036EA4] = t_record


def t_cur3(r):
    return Case([r.choice([0, 0, 1, 0x80000000, rnd(r)])],
                writes([(SYS + 0x3DA534, i32(r.choice(WV))), (SYS + 0x3DA538, i32(r.choice(WV))),
                        (SYS + 0x3DA540, i32(r.choice(WV + [1, 100]))), (SYS + 8, s16(r.choice(HV)))]), ret=V)


TESTS[0x400367EC] = t_cur3


def t_c_delete(r):
    return Case([], writes(list_area(r, r.choice(PS), r.choice(CNT[:5])) + [(SYS + 0xFA6, ents(r))]), ret=V,
                stubs=(rec("adv", 0x40037418, 1), rec("remove", 0x40036F74, 1), rec("7020", 0x40037020, 1)) +
                tuple(rec("%08x" % x, x, 0) for x in (0x40035320, 0x40036C54, 0x40036D10, 0x40036B00, 0x40036BDC)))


TESTS[0x40037460] = t_c_delete


# ================================================================ batch 32: widgets, fonts, trace cursors
EA38, ECC4, EBA8, CE7C, EB70 = 0x4002EA38, 0x4002ECC4, 0x4002EBA8, 0x4002CE7C, 0x4002EB70


def draw_stubs(*extra):
    return (rec("colour", EA38, 1), rec("rect", ECC4, 4), rec("span", EBA8, 3), rec("text", CE7C, 3),
            rec("pixel", EB70, 2)) + extra


def xy(r):
    return r.choice([0, 5, 100, 1000, 0xFFFF, r.getrandbits(16), r.getrandbits(32)])


TESTS[0x40019AF4] = lambda r: Case([], rand_block(r, SYS + 0x3DD530, 0x60), ret=V)
TESTS[0x40019B8C] = lambda r: Case([xy(r), xy(r), r.choice([0, 1, 0x100, rnd(r)])],
                                   compose(rand_block(r, SYS + 0x3DD568, 0x10), rand_block(r, 0x42E68C70, 8),
                                           rand_block(r, 0x42E68BE0, 8)), ret=V,
                                   stubs=draw_stubs(rec("half", 0x4002DC40, 1), rec("font", 0x4002CCCC, 1)))
TESTS[0x40019C7C] = lambda r: Case([xy(r), xy(r)], rand_block(r, 0x409223EC, 4), ret=V, stubs=draw_stubs())
TESTS[0x40019CF8] = lambda r: Case([xy(r), xy(r), r.choice([0, 1, 2, 3, 24, 25, 97, 98, 100, 255, r.getrandbits(8)])],
                                   rand_block(r, SYS + 0x3DD578, 12), ret=V, stubs=draw_stubs())


def t_battery(r):
    return Case([], compose(rand_block(r, 0x42E6455E, 8),
                            writes([(SYS + 0x778, bytes([r.choice([0, 1, 5, 12, r.getrandbits(8)]),
                                                         r.choice([0, 0, 1, r.getrandbits(8)])]))])),
                ret=V, stubs=draw_stubs(rec("frame", 0x4002EC4C, 4)))


TESTS[0x4001C200] = t_battery
TESTS[0x40023D04] = lambda r: Case([], compose(rand_block(r, 0x42E64630, 8), rand_block(r, 0x409223B8, 4)), ret=V,
                                   stubs=draw_stubs(rec("seg", 0x4002ED90, 4), rec_stk("btn", 0x4002F160, 1)))
TESTS[0x40025D08] = lambda r: Case([], compose(rand_block(r, 0x42E6465A, 8), rand_block(r, 0x40095498, 4)), ret=V,
                                   stubs=(rec_stk("btn", 0x4002F160, 1), rec("copy", 0x40035728, 3)))
TESTS[0x4002CCD8] = lambda r: Case([], rand_block(r, 0x42E64A98, 0x2700), ret=V, stubs=(rec("fmb", 0x4002DCAC, 1),))

FD = 0x42E66258        # a font descriptor
GD = SYS + 0x200000    # glyph data used by the tests


def t_metrics(r):
    a = r.choice([0, 1, 2, 5, 40, 150, r.randrange(0, 160)])
    data = bytes([a >> 8, a & 0xFF, r.getrandbits(8), r.getrandbits(8)]) + \
        bytes(r.choice([0, 0, 1, r.getrandbits(8)]) for _ in range(0x100))
    return Case([FD], writes([(FD, bytes(r.getrandbits(8) for _ in range(0x4BC))), (FD + 0x4BC, i32(GD)),
                              (GD, data)]), ret=V)


TESTS[0x4002DCAC] = t_metrics


def t_glyph(r):
    code = r.choice([(r.randrange(0xA1, 0xF8) << 8) | r.randrange(0xA1, 0xFF), 0x4141, 0x4142,
                     (r.randrange(0xB0, 0xD8) << 8) | r.randrange(0xA1, 0xFF)])
    return Case([xy(r), xy(r), code], ret=V, stubs=draw_stubs())


TESTS[0x4002CDC4] = t_glyph


def t_shadow(r):
    m = (r.getrandbits(24) << 8) | r.choice([0, 1, 2, 3])
    a = r.getrandbits(32)
    return Case([xy(r), xy(r), rnd(r), r.choice([0, 1, 2, 3])], rand_block(r, 0x42E6865C, 12), ret=V,
                stubs=draw_stubs(), stack=[m, a])


TESTS[0x4002D0D4] = t_shadow


def t_char(r):
    ch = r.choice([0, 1, 5, 19, 0x10000 | r.randrange(20)])
    hdr = bytes(r.getrandbits(8) for _ in range(0x4C0))
    items = [(FD, hdr), (FD + 2, s16(r.choice([0, 1, 7, 30]))), (FD + 4, s16(r.choice([0, 1, 2, 3, 6]))),
             (FD + 0xA, b"".join(s16(r.randrange(0, 100)) for _ in range(20))),
             (FD + 0x262, b"".join(s16(r.choice([0, 1, 3, 8])) for _ in range(20))),
             (FD + 0x4BC, i32(GD)), (GD, bytes(r.choice([0, 0, 1, 0xFF]) for _ in range(0x200))),
             (0x42E68C74, i32(FD))]
    return Case([xy(r), xy(r), ch], writes(items), ret=V, stubs=draw_stubs())


TESTS[0x4002E6D8] = t_char
TESTS[0x4002E7B0] = lambda r: Case([r.choice([r.randrange(0x20, 0x40), r.randrange(0x20, 0x40), rnd(r),
                                              0x100 + 0x30])], ret=R)


def t_blend(r):
    x, y = r.randrange(0, 1022), r.randrange(0, 8)
    fb = SYS + 0x100000
    return Case([x, y], compose(writes([(0x40095498, i32(fb)), (fb + 2 * (x + 1022 * y), s16(r.getrandbits(16)))]),
                                rand_block(r, 0x42E6865C, 12)), ret=V)


TESTS[0x4002EA88] = t_blend


def t_rect_copy_to(r):
    x0 = r.randrange(0, 1000)
    x1 = r.choice([x0, x0 + r.randrange(1, 40), max(0, x0 - 3)])
    y0 = r.randrange(0, 600)
    y1 = r.choice([y0, y0 + r.randrange(1, 6), max(0, y0 - 2)])
    rows = []
    xa, xb = max(0, min(x0, x1) - 1), max(x0, x1) + 1
    for y in range(max(0, min(y0, y1) - 1), max(y0, y1) + 2):
        for base in (0x41FB8FE8, 0x420E4E84):
            rows.append((base + 2 * (xa + 1022 * y), bytes(r.getrandbits(8) for _ in range(2 * (xb - xa)))))
    return Case([x0, y0, x1, y1], writes(rows), ret=V, stubs=(rec("dc", 0x4003E6F4, 4),))


TESTS[0x400342FC] = t_rect_copy_to


def t_repeat(r):
    tick = r.getrandbits(32)
    lim = r.choice([0, 1, 100, 1000, 0xFFFFFFFF])
    st = [(tick - r.choice([0, 1, 99, 100, 101, 999, 1000, 5000])) & 0xFFFFFFFF for _ in range(2)]
    return Case([], writes([(SYS + 0x86E, s16(r.choice([0, 1, 2, 3, 0xFFFF, r.getrandbits(16)])) +
                             s16(r.choice([0, 1, 2, 3, 0xFFFF, r.getrandbits(16)])) +
                             bytes([r.choice([0, 1, 0x80]), r.choice([0, 1, 0x80]), 0, 0])),
                            (SYS + 0x874, i32(st[0]) + i32(st[1]) + i32(lim)), (0x42E68CD8, i32(tick))]), ret=V)


TESTS[0x40032D3C] = t_repeat
LV = [-0x80000000, -600, -11, -1, 0, 9, 10, 11, 300, 500, 501, 0x7FFFFFFF]


def t_cursor_limits(r):
    g = b"".join(i32(r.choice([3, 4, 0, 5]) if i == 0 else r.choice(LV)) for i in range(6))
    q = b"".join(i32(r.choice(LV)) for _ in range(8))
    return Case([], compose(t17(r), writes([(0x42E180C8 + 0x48, g), (0x42E42884, q)])), ret=V,
                stubs=(rec("gen", 0x4004CF34, 4),))


TESTS[0x40037944] = t_cursor_limits
TESTS[0x40037B48] = lambda r: Case([], compose(rand_block(r, SYS, 0x700), rand_block(r, 0x42E554B0, 0x400),
                                               rand_block(r, 0x42E3F984 + 0x2EF0, 0x20),
                                               rand_block(r, TOUCH_BLK + 0x10, 8), t17(r)), ret=V)


def t_chain(r):
    b = bytes(r.choice([0, 1, 2]) for _ in range(4))
    nv = r.choice([0, 1, 2])

    def chg(m, a):
        m.calls.append(("9254",))
        m.w8(0x42E180C8 + 0x47, nv)
    return Case([], writes([(0x42E180C8 + 0x44, b)]), ret=V, stubs=tuple(rec("%08x" % a, a, 0) for a in (
        0x40037944, 0x40037B48, 0x4003A300, 0x40038374, 0x4003963C, 0x40039918, 0x400381E8, 0x40037C28)) +
        (Stub("9254", 0x40039254, chg, 0), rec("inv", 0x4003CD44, 2)))


TESTS[0x40037AB8] = t_chain


# ================================================================ batch 33: drawing primitives, buttons, markers
FB_T = SYS + 0x100000     # drawing target used by these tests
CBLK = 0x42E6865C


def colour(r, alpha=None):
    a = alpha if alpha is not None else r.choice([0, 0, 1, 0x80, 0xFF])
    return writes([(CBLK, bytes([a]) + bytes(r.getrandbits(8) for _ in range(11)))])


def t_vline(r):
    x, y0 = r.randrange(0, 1022), r.randrange(0, 8)
    y1 = r.choice([y0, y0 + r.randrange(1, 6), y0 - 1, y0 - 3, 0xFFFE0000 + y0])
    return Case([x, y0, y1 & 0xFFFFFFFF], compose(writes([(0x40095498, i32(FB_T))]), colour(r)), ret=V,
                stubs=(rec("pixel", EB70, 2),))


def t_rect(r):
    x, y = r.randrange(0, 1000), r.randrange(0, 8)
    w, h = r.choice([0, 1, 2, 5, 20, 0xFFFFFFFF]), r.choice([0, 1, 2, 4, 0xFFFFFFFE])
    return Case([x, y, w, h], compose(writes([(0x40095498, i32(FB_T))]), colour(r)), ret=V,
                stubs=(rec("blend", 0x4002EA88, 2), rec("fill", 0x400356F4, 3)))


TESTS[0x4002EBC8] = t_vline
TESTS[0x4002ECC4] = t_rect
TESTS[0x4002EC4C] = lambda r: Case([xy(r), xy(r), xy(r), xy(r)], ret=V, stubs=(rec("rect", ECC4, 4),))


def t_line(r):
    x0, y0 = r.randrange(0, 900), r.randrange(0, 500)
    x1 = r.choice([x0, x0 + r.randrange(1, 30), x0 - r.randrange(1, 30)])
    y1 = r.choice([y0, y0 + r.randrange(1, 300), y0 - r.randrange(1, 300)])
    return Case([x0, y0, x1, y1], ret=V, stubs=(rec("vline", 0x4002EBC8, 3),))


TESTS[0x4002ED90] = t_line


def t_d6f8(r):
    st = [(r.getrandbits(24) << 8) | r.choice([0, 1, 2, 3]) for _ in range(3)] + [r.getrandbits(32)]
    st[0] = r.getrandbits(32)
    return Case([xy(r), xy(r), rnd(r), rnd(r)], rand_block(r, CBLK, 12), ret=V, stack=st,
                stubs=(rec("colour", EA38, 1), rec_stk("txt", 0x4002D6F8, 1)))


TESTS[0x4002D724] = t_d6f8


def t_tint(r):
    c = t_char(r)
    c.setup = compose(c.setup, colour(r))
    c.stubs = draw_stubs()
    return c


TESTS[0x4002E878] = t_tint


def t_msgbox(r):
    wd = r.choice([0, 10, 100, 400, 0xFFFF, 0xFFFFFFF0])
    return Case([r.choice([0, 5, 30, 200, 800, 0xFFFFFFFB, 0xFFFF]), r.choice([0, 10, 24, 25, 60, 300, 0xFFFFFFFF]), rnd(r),
                 r.choice([0, 1, 2, 3, 0xFF])], compose(rand_block(r, 0x42E68C38, 8), rand_block(r, 0x409223BC, 4)),
                ret=V, stubs=(rec("font", 0x4002CCCC, 1), rec("half", 0x4002DC40, 1),
                              rec("width", 0x4002D164, 1, ret=wd), rec("save", 0x400342FC, 4),
                              rec("back", 0x400342DC, 0), rec("colour", EA38, 1), rec("rect", ECC4, 4),
                              rec_stk("text", 0x4002D0D4, 2), rec("present", 0x400343C4, 4)))


TESTS[0x40034134] = t_msgbox
TBU = 0x4002F2BC
TESTS[0x4002F968] = lambda r: Case([], writes([(0x42E3F984 + 0xC, bytes([r.choice([0, 0, 1, 0x80])]))]), ret=V,
                                   stubs=(rec("tbu", TBU, 1),))
for _a in (0x4002FAAC, 0x4002FB98, 0x4002FC38):
    TESTS[_a] = lambda r: Case([], writes([(SYS + 0xF9A, s16(r.choice([0, 1, 2, 5, 12, 20])))]), ret=V,
                               stubs=(rec("tbu", TBU, 1),))
TESTS[0x4002FDCC] = lambda r: Case([], ret=V, stubs=(rec("tbu", TBU, 1),))


def t_marker(r):
    vals = [r.choice([0, 0, 1, -1, 5, -5, 100, r.getrandbits(16) - 0x8000]) for _ in range(24)]
    v = r.choice(vals + [0, 1, -1, 5, -5, 100, r.getrandbits(32)])
    return Case([r.choice([0, 1, 2, 0xFF]), v & 0xFFFFFFFF],
                writes([(SYS + 0x800, bytes([r.getrandbits(8), r.choice([0, 0xFF, r.getrandbits(8)])])),
                        (SYS + 0x880, b"".join(s16(x) for x in vals))]), ret=V)


TESTS[0x40030060] = t_marker


def t_minmax(r):
    buf = SYS + 0x180000
    e = r.choice([0, 1, 2, 10, 40, 100])
    s0 = r.choice([0, 0, 1, e, e + 2])
    data = bytes(r.getrandbits(8) for _ in range(0x100))
    return Case([r.choice([0, 1, 2, 3])], writes([(SYS + 0x7B4, i32(buf) + i32(buf + 0x10) + i32(buf) + i32(buf)),
                                               (SYS + 0x2B0, i32(s0) + i32(e)),
                                               (SYS + 0x3E, bytes([r.choice([0, 1])])),
                                               (SYS + 0x102, bytes([r.choice([0, 1])])),
                                               (SYS + 0xB60, bytes(r.getrandbits(8) for _ in range(0x38))),
                                               (buf, data)]), ret=V)


TESTS[0x40032A58] = t_minmax


# ================================================================ batch 34: view, zoom, MMU, FFT, boot steps
I64 = lambda v: (v & 0xFFFFFFFFFFFFFFFF).to_bytes(8, "little")
OFFV = [-0x80000000, -2000, -5, -1, 0, 1, 2, 500, 1021, 1022, 1023, 5000, 0x7FFFFFFF]


def t_view(r):
    return Case([], writes([(SYS + 0x620, i32(r.choice(OFFV)) + i32(r.choice(OFFV))),
                            (SYS + 0xC, s16(r.choice([0, 1, 10, 0xFFFF]))),
                            (SYS + 8, s16(r.choice([0, 1, 2, 600, 1022, 0xFFFF]))),
                            (SYS + 0x10, bytes(r.getrandbits(8) for _ in range(0x10)))]), ret=V)


TESTS[0x40042280] = t_view


def t_zoom(r):
    seq = [r.choice([0, 1, 50, 1000, 10 ** 6, 10 ** 9, 1 << 40, (1 << 64) - 1]) for _ in range(r.randrange(0, 6))]
    den = r.choice([1, 2, 10, 1000, 1 << 33])
    per = r.choice([1, 3, 100])

    def sel(m, a):
        m.calls.append(("sel",))
        m.write(SYS + 0x590, I64(m.seq.pop(0) if m.seq else 0))

    def setup(m):
        m.seq = list(seq)
    return Case([], compose(writes([(SYS + 0x588, bytes([r.choice([0, 1, 5, 20, 0xFF])])),
                                    (SYS + 0x590, I64(r.getrandbits(64)) + bytes([r.choice([0, 1, 4, 30, 0xFF])]) +
                                     bytes(7) + I64(den)),
                                    (SYS + 0x5D4, i32(per)), (SYS + 0x2AC, s16(r.choice([1, 2, 100, 0xFFFF]))),
                                    (SYS + 0x2A7, bytes([r.choice([0, 3, 10, 0xFF]), r.choice([0, 1, 4, 12])]))]),
                            setup), ret=V,
                stubs=(Stub("sel", 0x40041FF8, sel, 0), rec("inv", 0x4003CD44, 2)))


TESTS[0x400423B8] = t_zoom


def t_mmu(r):
    tbl = SYS + 0x200000
    va = r.choice([0, 0x40000000, 0x01C00000, r.getrandbits(32)]) & 0x0FF00000
    pa = r.choice([0, 0x40000000, r.getrandbits(32), r.getrandbits(40)])
    size = r.choice([0, 0xFFFFF, 0x100000, 0x300000, 0x1000000, 0x1FFFFFF, (1 << 32) + 0x200000])
    size = size if size < (1 << 32) else (r.choice([0, 1]) << 32) | 0x200000
    return Case([tbl, 0, va & 0xFFFFFFFF, va >> 32], writes([(tbl + (va >> 20) * 4 - 8,
                                                               bytes(r.getrandbits(8) for _ in range(0x100)))]),
                ret=V, stack=[pa & 0xFFFFFFFF, pa >> 32, size & 0xFFFFFFFF, size >> 32, r.getrandbits(32)])


TESTS[0x400429E4] = t_mmu
TESTS[0x4004EC98] = lambda r: Case([r.choice([0, 1, 2, 7, 100])], ret=V,
                                   stubs=(rec("sel", 0x4004927C, 1), rec("wr", 0x40049214, 1)))


def t_calres(r):
    rc = r.choice([1, 0, -1, -2, -3, 2, 5]) & 0xFFFFFFFF
    nb = bytes(r.getrandbits(8) for _ in range(3))

    def cal(m, a):
        m.calls.append(("cal",))
        m.write(0x42E68B01, nb)
        return rc
    return Case([], rand_block(r, 0x42E68AE0, 0x30), ret=V, stubs=(Stub("cal", 0x4004C6B4, cal, 0),))


TESTS[0x4004FAB0] = t_calres


def t_fft_in(r):
    n = r.choice([0, 1, 2, 7, 32])
    total = r.choice([0, 1, n, 2 * n + 1, 5 * n + 3, 100])
    return Case([], compose(writes([(SYS + 0x7F8, i32(n)), (SYS + 0x454, i32(total)),
                                    (SYS + 0x264, i32(r.choice([0, 1, 4, 15, 31, 32, 40, 0x101]))),
                                    (SYS + 0x3E, bytes([r.choice([0, 1])])), (SYS + 0x42, bytes([r.choice([0, 1, 1])])),
                                    (SYS + 0x102, bytes([r.choice([0, 1])])),
                                    (SYS + 0x106, bytes([r.choice([0, 1, 1])]))]),
                            rand_block(r, 0x42AC7CF4, 0x200), rand_block(r, 0x42AC7CF4 + 0x62250, 0x200),
                            rand_block(r, 0x42E49EB4, 0x120), rand_block(r, 0x42E4DEB4, 0x120)), ret=V)


TESTS[0x40041A4C] = t_fft_in


def t_fft_draw(r):
    return Case([], compose(writes([(SYS + 4, s16(xy(r) & 0xFFFF) + s16(r.randrange(0, 300)) +
                                     s16(r.choice([1, 2, 5, 30])) + s16(r.randrange(0, 600))),
                                    (SYS + 0x3E, bytes([r.choice([0, 1])])), (SYS + 0x42, bytes([r.choice([0, 1, 1])])),
                                    (SYS + 0x102, bytes([r.choice([0, 1])])),
                                    (SYS + 0x106, bytes([r.choice([0, 1, 1])]))]),
                            rand_block(r, 0x42E5B014, 0x40), rand_block(r, 0x42E5C014, 0x40)), ret=V,
                stubs=(rec("colour", EA38, 1), rec("vline", 0x4002EBC8, 3)))


TESTS[0x40041E34] = t_fft_draw


def t_diskchk(r):
    rdata = bytes(r.getrandbits(8) for _ in range(0x1E))
    same = r.random() < 0.5

    def wr(m, a):
        m.calls.append(("write", a[0] == m.fil, bytes(m.read(a[1], a[2])), a[2], a[3]))
        m.wdata = bytes(m.read(a[1], a[2]))
        return 0

    def rd(m, a):
        m.write(a[1], m.wdata if same else rdata)
        m.calls.append(("read", a[0] == m.fil, a[2], a[3]))
        return 0

    def op(m, a):
        m.fil = a[0]
        m.calls.append(("open", a[1], a[2] & 0xFF))
        return 0

    def cl(m, a):
        m.calls.append(("close", a[0] == m.fil))
        return 0

    def ls(m, a):
        m.calls.append(("lseek", a[0] == m.fil, a[2], a[3]))
        return 0
    return Case([], rand_block(r, 0x42E68AE0, 0x20), ret=V,
                stubs=(rec("fmt", 0x40051C04, 0), quiet("f_open", F_OPEN, op, 3), quiet("f_close", F_CLOSE, cl, 1),
                       quiet("f_lseek", F_LSEEK, ls, 4), quiet("f_write", F_WRITE, wr, 4),
                       quiet("f_read", F_READ, rd, 4), rec("unlink", F_UNLINK, 1)))


TESTS[0x40051C28] = t_diskchk
FWORDS = [(0, 0x5A5555A5), (4, 0xA5AA555A), (8, 0x5A5AA5AA), (0xC, 0xA5A5AA55)]


def t_factory(r):
    blk = b"".join(i32(v if r.random() < 0.5 else r.getrandbits(32)) for _, v in FWORDS)
    return Case([], writes([(0x42E68AE0, blk)]), ret=V, stubs=tuple(rec("%08x" % a, a, 0) for a in (
        0x40050FE0, 0x40051578, 0x400519E4, 0x40043E00, 0x400447C8, 0x40044794, 0x4003EEB0, 0x4003E708, 0x4004D18C)))


TESTS[0x40051CF0] = t_factory


def win_stack(r):
    n = r.choice([0, 1, 2, 5])
    return writes([(0x42E68BC8, bytes([r.getrandbits(8), n]) + bytes(r.choice([0, 1, 1, 2, 30]) for _ in range(8)))])


def t_shot(r):
    st = [r.choice([0, 1, 2, 2]) for _ in range(10)]

    def tbu(m, a):
        m.calls.append(("tbu", a[0]))
        m.w8(0x42E62040, m.st.pop(0) if m.st else 0)

    def setup(m):
        m.st = list(st)
    return Case([], compose(win_stack(r), rand_block(r, 0x42E61FF8, 0x60), setup), ret=V,
                stubs=(Stub("tbu", 0x4002F2BC, tbu, 1), rec("shot", 0x4003506C, 0)))


TESTS[0x40051E24] = t_shot
TESTS[0x40052098] = lambda r: Case([], compose(win_stack(r), rand_block(r, 0x42E64520, 0x1C0),
                                               rand_block(r, TOUCH_BLK + 8, 8), t17(r)), ret=V,
                                   stubs=(rec("shot", 0x40051E24, 0), rec("touch", 0x40051E88, 1)))


def t_curvals(r):
    w, h = r.choice([1, 2, 100, 1022, 0xFFFF]), r.choice([1, 2, 600, 0xFFFF])
    c1, c2 = r.randrange(0, 40), r.randrange(0, 40)
    f64 = lambda v: __import__("struct").pack("<d", v)
    return Case([], compose(rand_block(r, 0x42E180C8 + 0x8D20, 0x60), rand_block(r, 0x42E180C8 + 0x9CC0, 0x60),
                            writes([(0x42E180C8 + 4, s16(w) + s16(h)),
                                    (0x42E180C8 + 0x10, b"".join(f64(r.uniform(-10, 10)) for _ in range(4))),
                                    (0x42E180C8 + 0x8D08, f64(r.uniform(-3, 3)) + bytes(8) + f64(r.uniform(-2, 2))),
                                    (0x42E180C8 + 0xAC64, i32(c1) + i32(c2))]), t17(r)), ret=V)


TESTS[0x400381E8] = t_curvals


# ================================================================ batch 35: button handlers, counters, levels
BT35 = 0x42E61FF8
WCLOSE, WOPEN = 0x4003CE50, 0x4003CDFC


def btn_states(r, offs, vals=(0, 1, 2, 2, 3)):
    return writes([(BT35, bytes([r.choice([0, 1, 5])]))] + [(BT35 + o, bytes([r.choice(vals)])) for o in offs])


def t_btn(offs, callees, vals=(0, 1, 2, 2, 3), extra=None):
    def t(r):
        setup = btn_states(r, offs, vals)
        if extra:
            setup = compose(setup, extra(r))
        return Case([], setup, ret=V, stubs=tuple(rec("%08x" % a, a, n) for a, n in callees))
    return t


TESTS[0x40055344] = t_btn([0x912], [(0x4002FC30, 0), (0x400361CC, 0), (WCLOSE, 1), (0x400365F4, 0)])
TESTS[0x40054EB8] = t_btn([0x7FA], [(0x4002FB40, 0), (0x400350F4, 0), (WCLOSE, 1), (0x40035578, 0), (0x40035624, 0)])
TESTS[0x400555E8] = t_btn([0x9EE], [(0x4002FCCC, 0), (0x40037460, 0), (WCLOSE, 1)])
TESTS[0x40055720] = t_btn([0x106, 0x110, 0x11A], [(0x4002FD10, 0), (WOPEN, 1)], vals=(0, 1, 2, 3, 3))
TESTS[0x40035540] = lambda r: Case([], rand_block(r, SYS + 0xF88, 0x20), ret=V, stubs=tuple(
    rec("%08x" % a, a, 0) for a in (0x4003486C, 0x40035320, 0x40035248, 0x400354C0, 0x40035578)))
TESTS[0x40052F08] = t_btn([0x340, 0x34A, 0x354], [(0x4002F5EC, 0), (0x4003CF0C, 0), (WOPEN, 1), (0x40035540, 0),
                                                  (0x40035744, 0), (0x400378E0, 0)], vals=(0, 1, 2, 3, 3))
TESTS[0x40055AD4] = t_btn([0x156, 0x160, 0x16A, 0x174], [(0x4002FD80, 0), (0x400457B0, 0), (0x40043A80, 0),
                                                         (0x4003CDC0, 0)], vals=(0, 1, 2, 3, 3, 3),
                          extra=lambda r: writes([(SYS + 0x7AC, bytes([r.getrandbits(8)]))]))
TESTS[0x40055634] = t_btn([0x9F8, 0xA02, 0xA0C, 0xA16], [(0x4002FCD4, 0), (0x400373E0, 0), (0x400367D0, 0),
                                                         (WCLOSE, 1), (WOPEN, 1), (0x4003CD44, 2)],
                          vals=(0, 1, 2, 3, 3, 3), extra=lambda r: writes([(0x4091F398, bytes([r.getrandbits(8)]))]))


def win_rect(r, off):
    return writes([(0x42E64520 + off, b"".join(s16(r.choice([0, 1, 100, 1021, 0xFFFF, r.getrandbits(16)]))
                                                for _ in range(4)))])


def msg_texts(r, base):
    return compose(rand_block(r, 0x42C2FE74 + base + 0x190, 0x8), rand_block(r, 0x42C2FE74 + base + 0x330, 0x8),
                   writes([(0x42C2FE74 + base, i32(r.getrandbits(32))),
                           (0x42C2FE74 + base + 0x1A0, i32(r.getrandbits(32)))]))


TESTS[0x40055BD4] = t_btn([0x200], [(0x4002FDBC, 0), (0x40049BB4, 0), (0x40034134, 4), (0x40042CF0, 1),
                                    (0x40034284, 0), (WCLOSE, 1)],
                          extra=lambda r: compose(win_rect(r, 0x1B8), msg_texts(r, 0xAAA0)))
TESTS[0x40056974] = t_btn([0xC28], [(0x40030008, 0), (0x40034134, 4), (0x40060D68, 0), (0x40042CF0, 1), (WCLOSE, 1)],
                          extra=lambda r: compose(win_rect(r, 0x252), msg_texts(r, 0x9C00)))
TESTS[0x40054540] = t_btn([0x71E, 0x71F], [(0x4002FAA4, 0), (0x4006904C, 0), (0x40069084, 0)], vals=(0, 1, 1, 2, 7),
                          extra=lambda r: compose(rand_block(r, 0x42E68998, 0x40), rand_block(r, SYS + 0x27E, 2),
                                                  writes([(0x42E68998, bytes([r.choice([0x12, 0x12, 0x11])]))])))
TESTS[0x40063484] = lambda r: Case([], writes([
    (SYS + 0x1AE, s16(r.choice([0, 1, 20, 300, 0x7FFF, 0xFFFF]))), (SYS + 0xA, s16(r.choice([0, 1, 100, 600, 0xFFFF]))),
    (SYS + 0x27E, s16(r.choice([0, 1, 19, 20, 21, 40, 300, 580, 600, 0x7FFF, 0x8000, 0xFFFF, r.getrandbits(16)]))),
    (SYS + 0x2AA, bytes([r.choice([0, 1, 9])]))]), ret=V)
TESTS[0x40063BC0] = lambda r: Case([], compose(rand_block(r, SYS + 0x670, 0x10), rand_block(r, SYS + 0xBBC, 0x10)),
                                   ret=V, stubs=(rec("sel", SEL, 1), rec("wr", WR, 1)))


def t_cstep(r):
    ch = r.choice([0, 1, 3, 7, 0xFF])
    cur = r.choice([0, 1, 5, 0x80, 0xFF])
    old = r.choice([cur, 0, 3, 0x7F, 0xFF])
    return Case([ch], compose(rand_block(r, SYS + 0x678, 0x110), rand_block(r, SYS + 0xBBC, 0x110),
                              writes([(SYS + 0x67C + ch, bytes([cur])), (SYS + 0xBC0 + ch, bytes([old]))])),
                ret=V, stubs=(rec("sel", SEL, 1), rec("wr", WR, 1)))


TESTS[0x40063C34] = t_cstep
LVSRC = SYS + 0x200000


def lv_common(r):
    few = b"".join(s16(r.choice([0, 0, 1, 0xFFFF, 2, 299, 300, 301, 0x7FFF, 0x8000, r.getrandbits(16)]))
                   for _ in range(0x10))
    return compose(rand_block(r, LVSRC, 0x100), writes([(LVSRC, few), (0x42E68CB0, i32(LVSRC))]))


TESTS[0x400621EC] = lambda r: Case([], compose(lv_common(r), writes([
    (0x42E68668 + 0xB8, i32(r.choice([0, 1, 5, 64, -1]))),
    (0x42E68668 + 0xC0, i32(r.choice([0, 1, 2, 300, 0x7FFF, -5, 0x40000000])))])), ret=V)
TESTS[0x40062284] = lambda r: Case([], compose(lv_common(r), writes([
    (0x42E68668 + 1, bytes([r.choice([0, 1, 1, 1])])),
    (0x42E68668 + 0xDC, i32(r.choice([0, 1, 5, 64, -1]))),
    (0x42E68668 + 0xE0, i32(r.choice([0, 1, 300, 0x7FFF, -7, 0x10000]))),
    (0x42E68668 + 0xC0, i32(r.choice([0, 1, -1, 2, 300, 0x8000, -300])))])), ret=V)


def t_fftb(r):
    n = r.choice([0, 1, 2, 7, 32])
    return Case([], compose(rand_block(r, 0x42AC7CF4, 0x200), rand_block(r, 0x42D164B4, 0x120),
                            writes([(0x42E68668 + 0xF8, i32(n)),
                                    (SYS + 0x454, i32(r.choice([0, n, 2 * n + 1, 100, 300]))),
                                    (SYS + 0x264, i32(r.choice([0, 1, 4, 15, 31, 32, 40, 0x101])))])), ret=V)


TESTS[0x40061580] = t_fftb


def t_pos290(r):
    flag = r.choice([0, 1, 2])
    vals = [r.getrandbits(8) for _ in range(2)]

    def cmd18(m, a):
        m.calls.append(("cmd18",))
        m.w8(SYS + 0x279, flag)

    def rd(m, a):
        m.calls.append(("rd",))
        return m.vals.pop(0) if m.vals else 0

    def setup(m):
        m.vals = list(vals)
    x = r.choice([0, 1, 100, 0x1FFF, r.getrandbits(32), ((vals[0] << 8) | vals[1]) & 0x1FFF])
    t = r.choice([0, x, x + 1, max(x - 1, 0), r.getrandbits(32), r.getrandbits(64)])
    return Case([], compose(writes([(SYS + 0x28C, i32(x)), (SYS + 0x30, I64(t)),
                                    (SYS + 0x520, i32(r.choice([1, 7, 0x2000, 0xFFFFFFFF, r.getrandbits(32) | 1])))]),
                            setup), ret=V,
                stubs=(Stub("cmd18", 0x40063770, cmd18, 0), Stub("rd", RD, rd, 0), rec("sel", SEL, 1)))


TESTS[0x40063798] = t_pos290


def t_meas(r):
    f64 = lambda v: __import__("struct").pack("<d", v)
    return Case([], writes([(0x42E68668, bytes(r.getrandbits(8) for _ in range(8))),
                            (0x42E68668 + 0x10, f64(r.uniform(-1e6, 1e6))),
                            (0x42E68668 + 0x20, f64(r.uniform(-1e6, 1e6)) + f64(r.uniform(-1e6, 1e6))),
                            (0x42E68668 + 0x48, f64(r.choice([0.0, 0.5, -0.5, 1.5, -3.7, r.uniform(-1e15, 1e15)])))]),
                ret=V, stubs=tuple(rec("%08x" % a, a, 0) for a in (0x400612FC, 0x40060FE8, 0x40060DC8, 0x40061F10)))


TESTS[0x40062E14] = t_meas


# ================================================================ batch 36: more button handlers, levels, FPGA params
def drag_blk(r, modes):
    return compose(rand_block(r, 0x42E68998, 0x40), writes([(0x42E68998, bytes([r.choice(modes)]))]))


TESTS[0x40056214] = t_btn([0xB42, 0xB43], [(0x4002FEE8, 0), (0x400692A0, 0), (0x40069324, 0), (0x40037A60, 0)],
                          vals=(0, 1, 1, 2), extra=lambda r: compose(drag_blk(r, [0xD, 0xE, 0xE, 0xF, 0xF, 0x10]),
                                                                     rand_block(r, 0x42E22D2C, 8)))
TESTS[0x40056724] = t_btn([0xBD8, 0xBD9], [(0x4002FFA4, 0), (0x40069150, 0), (0x400691D4, 0)], vals=(0, 1, 1, 2),
                          extra=lambda r: compose(drag_blk(r, [0xF, 0x10, 0x10, 0x11, 0x11, 0x12]),
                                                  rand_block(r, 0x42E68668 + 0x58, 0x10)))
TESTS[0x400562F4] = t_btn([0xB4C], [(0x4002FEF0, 0), (0x40034134, 4), (0x40039EB8, 0), (0x40042CF0, 1), (WCLOSE, 1)],
                          extra=lambda r: compose(win_rect(r, 0x20C), msg_texts(r, 0x9C00), msg_texts(r, 0xD680),
                                                  writes([(0x42E180C8 + 0xAD19, bytes([r.choice([0, 1]), r.choice([0, 1])]))])))
TESTS[0x40056404] = t_btn([0xB56, 0xB60, 0xB6A, 0xB74, 0xB7E, 0xB88],
                          [(0x4002FEF8, 0), (0x4003F064, 0), (0x4003CF0C, 0), (0x40034454, 0), (0x4003D6AC, 0), (WOPEN, 1),
                           (0x4003506C, 0), (0x40062E00, 0)], vals=(0, 1, 2, 3, 3, 3, 3),
                          extra=lambda r: compose(rand_block(r, SYS + 0x54C, 4), rand_block(r, SYS + 0x5F0, 4),
                                                  writes([(0x42E68668, bytes([r.choice([0, 1, 7])])),
                                                          (0x42E68668 + 0x58, i32(r.choice([0, 1, 0x100])))])))
TESTS[0x400524B4] = t_btn([0xAC, 0xB6, 0xC0, 0xCA, 0xD4, 0xDE, 0xE8], [(0x4002F3C4, 0), (WOPEN, 1), (0x4003CD44, 2)],
                          vals=(0, 1, 2, 3, 3, 3, 3, 3),
                          extra=lambda r: writes([(SYS + 0x671, bytes([r.choice([0, 1, 5])]))]))
TESTS[0x40055F2C] = t_btn([0xAC0, 0xACA, 0xAD4, 0xADE, 0xAF2, 0xAFC],
                          [(0x4002FE3C, 0), (0x4003F064, 0), (0x4003F088, 0), (0x4003CF0C, 0), (0x40034454, 0),
                           (0x4003D6AC, 0), (WOPEN, 1), (0x4003506C, 0)], vals=(0, 1, 2, 3, 3, 3, 3),
                          extra=lambda r: compose(rand_block(r, 0x42E180C8 + 0x40, 0x10),
                                                  writes([(0x42E180C8 + 0xAC60, bytes([r.choice([0, 1, 9])]))])))


def t_syscal(r):
    rc = r.choice([-3, -2, -1, 0, 1, 2]) & 0xFFFFFFFF
    base = t_btn([0x188], [(0x4002FD78, 0), (0x40034134, 4), (0x40034284, 0), (0x40042CF0, 1), (WCLOSE, 1),
                           (0x4003CD44, 1)],
                 extra=lambda r: compose(win_rect(r, 0x19C), msg_texts(r, 0x9C00),
                                         rand_block(r, 0x42C2FE74 + 0x9C00 + 0x4D0, 8),
                                         rand_block(r, 0x42C2FE74 + 0x9C00 + 0x670, 8),
                                         rand_block(r, 0x42C2FE74 + 0x9C00 + 0x810, 8),
                                         writes([(0x42C2FE74 + 0x9C00 + 0x340, i32(r.getrandbits(32))),
                                                 (0x42C2FE74 + 0x9C00 + 0x4E0, i32(r.getrandbits(32))),
                                                 (0x42C2FE74 + 0x9C00 + 0x680, i32(r.getrandbits(32)))])))(r)
    base.stubs = tuple(base.stubs) + (rec("cal", 0x4004C6B4, 0, rc),)
    return base


TESTS[0x40055978] = t_syscal
TESTS[0x40056804] = t_btn([0xBE2, 0xBEC, 0xBF6, 0xC00, 0xC0A, 0xC14, 0xC1E], [(0x4002FFAC, 0), (WOPEN, 1)],
                          vals=(0, 1, 2, 3, 3, 3, 3, 3, 3),
                          extra=lambda r: rand_block(r, 0x42E68668, 0x100))
TESTS[0x40056080] = t_btn([0xB06, 0xB10, 0xB1A, 0xB24, 0xB2E, 0xB38],
                          [(0x4002FE98, 0), (0x4005F9F0, 2), (0x4005FA78, 2), (0x4005FABC, 2)], vals=(0, 1, 2, 3, 3, 3, 3),
                          extra=lambda r: compose(rand_block(r, 0x42E68A88, 0x60), rand_block(r, 0x42E180C8 + 0x40, 0x20)))
TESTS[0x400623F4] = lambda r: Case([], compose(writes([(0x42E68668 + 4, bytes([r.choice([0, 1])]))]), t17(r)), ret=V,
                                   stubs=tuple(rec("%08x" % a, a, 0) for a in (
                                       0x40061DB0, 0x40061A70, 0x4006053C, 0x400621EC, 0x40062284, 0x4006233C)))
TESTS[0x400634F0] = lambda r: Case([], compose(rand_block(r, SYS + 0x1C0, 0x420), rand_block(r, SYS + 0x40, 0x10),
                                               rand_block(r, SYS + 0x5C, 8), rand_block(r, SYS + 0xDC, 8),
                                               rand_block(r, SYS + 0x100, 0x30), rand_block(r, SYS + 0x270, 0x14),
                                               rand_block(r, 0x42E53A0C + 0x10, 0x30),
                                               writes([(SYS + 0x27C, bytes([r.choice([0, 1])]))])), ret=V,
                                   stubs=(rec("clamp", 0x40063484, 0), rec("par", 0x40049EC4, 4), rec("smec", 0x40049F1C, 1),
                                          rec("cmd17", 0x40063628, 0)))


def t_gst(r):
    rets = [r.choice([0, 1, 1, 1, 2, 3, 0xFF]) for _ in range(4)]

    def chk(m, a):
        m.calls.append(("chk", a[0], a[1], a[2]))
        return m.rets.pop(0) if m.rets else 0

    def setup(m):
        m.rets = list(rets)
    return Case([], compose(rand_block(r, 0x42E3F984 + 0x2EF8, 0x10), setup), ret=R,
                stubs=(rec("gen", 0x4004D18C, 0), Stub("chk", 0x4006453C, chk, 3)))


TESTS[0x40064C28] = t_gst


# ================================================================ batch 37: output and menu buttons, numpad keys, tables
TESTS[0x400537C4] = t_btn([0x55C + 10 * i for i in range(9)], [(0x4002F86C, 0), (0x4003E708, 0), (0x4003CD44, 1)],
                          vals=(0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3),
                          extra=lambda r: writes([(SYS + 0x660, bytes([r.getrandbits(8)]))]))
TESTS[0x40052100] = t_btn([2, 0xC, 0x16, 0x20, 0x2A, 0x34, 0x3E, 0x48, 0x52],
                          [(0x4002F324, 0), (WOPEN, 1), (0x400360F0, 0), (0x400424D4, 0)],
                          vals=(0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3),
                          extra=lambda r: compose(rand_block(r, SYS + 0x800, 4), writes([
                              (SYS + 0x588, bytes([r.choice([0, 12, 13, 0xFF])])), (SYS + 0x2AA, bytes([r.choice([0, 1, 4])]))])))
TESTS[0x4006233C] = lambda r: Case([], compose(lv_common(r), writes([
    (0x42E68668 + 2, bytes([r.choice([0, 1, 1, 1])])),
    (0x42E68668 + 0xEC, i32(r.choice([0, 1, 5, 64, -1]))),
    (0x42E68668 + 0xF0, i32(r.choice([0, 1, 300, 0x7FFF, -7, 0x10000]))),
    (0x42E68668 + 0xC0, i32(r.choice([0, 1, -1, 2, 300, 0x8000, -300]))),
    (0x42E68668 + 0x188, i32(r.choice([0, 1, -1, 3, 10, -4])))])), ret=V)


def t_gst2(r):
    rets = [r.choice([0, 1, 1, 1, 2, 3, 0xFF]) for _ in range(4)]

    def chk(m, a):
        m.calls.append(("chk", a[0], a[1], a[2]))
        return m.rets.pop(0) if m.rets else 0

    def setup(m):
        m.rets = list(rets)
    return Case([], compose(rand_block(r, 0x42E3F984 + 0x2EF8, 0x10), setup), ret=R,
                stubs=(rec("gen", 0x4004D18C, 0), Stub("chk", 0x400647F4, chk, 3)))


TESTS[0x40064CF0] = t_gst2
TESTS[0x4005FE94] = lambda r: Case([r.choice(list(range(0, 18)) + [0xFFFFFFFF, 0x103])], compose(
    rand_block(r, 0x42E68A88, 0x20), writes([(0x42E68A89, bytes([r.choice([0, 1, 2, 3, 4, 5])])),
                                             (0x42E68A9F, bytes([r.choice([0, 9, 10, 0xFF]), r.choice([0, 0, 1]),
                                                                 r.choice([0, 1, 7])]))])), ret=V,
    stubs=(rec("sign", 0x4005FC04, 0), rec("back", 0x4005FBB4, 0), rec("app", 0x4005FB94, 1), rec("fc84", 0x4005FC84, 1),
           rec("close", 0x4005FB44, 0)))


def t_win64(r):
    v = lambda: r.choice([0, 1, -1, 5, 100, -100, 1 << 40, -(1 << 40), (1 << 63) - 1, -(1 << 63), r.getrandbits(64)])
    return Case([], writes([(0x42E68668 + 0x120, b"".join(I64(v()) for _ in range(4))),
                            (0x42E68668 + 0x18C, i32(r.choice([0, 1, 2, 3, 3, 4, 4, 5])))]), ret=V)


TESTS[0x40061680] = t_win64
TESTS[0x40063A50] = lambda r: Case([], rand_block(r, SYS + 0x680, 0x48), ret=V)


# ================================================================ batch 38: floating-point level functions
import struct as _st
F64 = lambda v: _st.pack("<d", v)
F32 = lambda v: _st.pack("<f", v)


def t_mkval(r):
    n = r.choice([1, 2, 100, 1022, -5])
    return Case([], compose(rand_block(r, 0x42E5F010, 0x100), writes([
        (0x42E68668 + 0x5C, i32(r.randrange(0, 100)) + i32(r.randrange(0, 100))),
        (0x42E68668 + 0xB8, i32(n)), (0x42E68668 + 0xCC, i32(r.choice([1, 30, 600, -3]))),
        (0x42E68668 + 0x120, I64(r.choice([0, 1000, -(10 ** 9), r.getrandbits(40)]))),
        (0x42E68668 + 0x138, I64(r.choice([0, 1, 10 ** 6, 10 ** 12, -500]))),
        (0x42E68668 + 0x140, F64(r.uniform(-40, 40)) + F64(r.uniform(-100, 100)))])), ret=V)


TESTS[0x4006053C] = t_mkval


def t_fftdb(r):
    n = r.choice([0, 1, 3, 16, -1])
    mags = b"".join(F32(r.choice([0.0, -1.0, r.uniform(0, 10), r.uniform(0, 1e6), r.uniform(1e-3, 1)]))
                    for _ in range(16))
    return Case([], compose(t17(r), writes([
        (0x42D364B4, mags), (0x42E68668 + 0xF8, i32(n)), (0x42E68CB0, i32(SYS + 0x200000)),
        (SYS + 0x20, s16(r.choice([0, 1, 10, 1000, 4096]))),
        (SYS + 0x50, I64(r.choice([0, 1, 10 ** 6, 10 ** 9, r.getrandbits(64)]))),
        (0x42E68668 + 0xCC, i32(r.choice([1, 30, 600, -3]))), (0x42E68668 + 0xD0, i32(r.choice([0, 1, 5, -7]))),
        (0x42E68668 + 0x140, F64(r.choice([10.0, -10.0, r.uniform(0.5, 40)])) + F64(r.uniform(-100, 100)))]),
        rand_block(r, SYS + 0x200000, 0x40)), ret=V)


TESTS[0x40061DB0] = t_fftdb


def t_shade(r):
    top = r.choice([0, 1, 5, 20, 60])
    return Case([], compose(t17(r), rand_block(r, 0x42E56C30, 271 * 4), writes([
        (0x42E5F010, b"".join(s16(r.choice([0, 1, 5, 10, 30, 0xFFFF, 0x7FFF])) for _ in range(16))),
        (0x42E68668 + 0xB0, i32(r.choice([0, 100, 0xFFFF])) + i32(r.choice([0, 50, 0x10000])) +
         i32(r.choice([0, 1, 5, 16, -1]))),
        (0x42E68668 + 0xC0, i32(top)),
        (0x42E68668 + 0x160, i32(r.choice([0, 3, top // 2, top, top + 5, -2]))),
        (0x42E68668 + 0x168, i32(r.choice([0, -3, 2, top - 1, top])))])), ret=V,
        stubs=(rec("colour", 0x4002EA38, 1), rec("px", 0x4002EB70, 2)))


TESTS[0x40062B1C] = t_shade



def t_fftsc(r):
    n = r.choice([0, 1, 3, 16])
    vals = lambda: b"".join(F32(r.choice([0.0, -1.0, 1.0, r.uniform(0, 2), r.uniform(1, 1e6), float("inf")]))
                            for _ in range(16))
    return Case([], compose(rand_block(r, 0x42E5B010, 0x40), rand_block(r, 0x42E5C010, 0x40), writes([
        (0x42E4BEB4, vals()), (0x42E4FEB4, vals()), (SYS + 0x7F8, i32(n)),
        (SYS + 0xA, s16(r.choice([0, 1, 100, 600, 0xFFFF]))),
        (SYS + 0x3E, bytes([r.choice([0, 1, 1])])), (SYS + 0x42, bytes([r.choice([0, 1, 1])])),
        (SYS + 0x102, bytes([r.choice([0, 1, 1])])), (SYS + 0x106, bytes([r.choice([0, 1, 1])]))])), ret=V)


TESTS[0x40041D38] = t_fftsc


def t_freq(r):
    ch = r.choice([0, 0, 1, 1, 2])
    buf = SYS + 0x200000
    thr = r.choice([0, 100, 500, 1000, 0xFFFF])
    per = r.choice([2, 5, 13, 40])
    samp = b"".join(s16(r.choice([thr, 0, 0xFFFF]) if r.random() < 0.1 else
                        ((1000 if (i // per) % 2 else 0) + r.randrange(0, 3))) for i in range(0x180))
    lo = r.choice([0, 10, 0x100])
    hi = lo + r.choice([0, 1, 50, 0x200])
    return Case([ch], compose(rand_block(r, SYS + 0xB68, 0x48), writes([
        (buf, samp), (SYS + 0x7B4 + 4 * ch, i32(buf)),
        (SYS + 0xB6C, i32(thr)), (SYS + 0xB94, i32(thr)),
        (SYS + 0x2B0, i32(lo) + i32(hi)),
        (SYS + 0x3E, bytes([r.choice([0, 1, 1])])), (SYS + 0x102, bytes([r.choice([0, 1, 1])])),
        (SYS + 8, s16(r.choice([0, 1, 10, 100]))), (SYS + 0x1C, s16(r.choice([1, 3, 1000]))),
        (SYS + 0x554, i32(r.choice([1, 7, 1000])) + i32(r.choice([0, 1, 5])) + i32(r.choice([1, 2, 1000]))),
        (SYS + 0x590, I64(r.choice([10 ** 9, 10 ** 12, r.getrandbits(40) + 10 ** 9])))])), ret=V)


TESTS[0x40032B5C] = t_freq


# ================================================================ batch 39: settings, list and trigger buttons
INV = 0x4003CD44
CH_CALLS = [(0x4003E708, 0), (0x4003D6E4, 0)]
CH_BTN = [0, 0xA, 0x14, 0x1E, 0x28, 0x32, 0x3C, 0x46, 0x50, 0x5A]
TESTS[0x40052FC4] = t_btn([0x35E + o for o in CH_BTN], [(0x4002F614, 0)] + CH_CALLS, vals=(0, 1, 2, 3, 3, 3, 3),
                          extra=lambda r: compose(rand_block(r, SYS + 0x3E, 0x10),
                                                  writes([(SYS + 0x652, bytes([r.choice([0, 0, 1, 0x80])]))])))
TESTS[0x400531B0] = t_btn([0x3C2 + o for o in CH_BTN], [(0x4002F684, 0)] + CH_CALLS, vals=(0, 1, 2, 3, 3, 3, 3),
                          extra=lambda r: compose(rand_block(r, SYS + 0x102, 0x10),
                                                  writes([(SYS + 0x652, bytes([r.choice([0, 0, 1, 0x80])]))])))


def list_blk(r, base):
    return writes([(SYS + 0xF90, bytes([r.choice([0, 0, 1, 7])]) + bytes(r.getrandbits(8) for _ in range(9))),
                   (SYS + 0xF9A, s16(r.choice([0, 1, 4, 6, 6, 0xFFFF]))),
                   (SYS + 0xF9E, s16(r.choice([0, 1, 3, 30, 0xFFFF]))),
                   (SYS + 0xFA0, s16(r.choice([0, 1, 5, 12, 40, 0xFFFF]))),
                   (SYS + 0xFA4, s16(r.choice([0, 1, 5, 7, 11, 40, 0xFFFF, 0x8000, 0x7FFF]))),
                   (SYS + base, bytes(r.choice([0, 0, 1, 9]) for _ in range(64)))])


TESTS[0x40054F50] = t_btn([0x818, 0x822, 0x82C, 0x836, 0x80E], [
    (0x4002FB50, 0), (INV, 1), (WCLOSE, 1), (WOPEN, 1), (0x40035578, 0), (0x40035320, 0), (0x400353A8, 0),
    (0x40035248, 0), (0x40035624, 0)], vals=(0, 1, 2, 3, 3, 3), extra=lambda r: list_blk(r, 0x5DC6 - 20))
TESTS[0x40055114] = t_btn([0x840 + 10 * i for i in range(7)] + [0x8D6, 0x8E0, 0x8EA, 0x8F4, 0x8FE, 0x908], [
    (0x4002FB98, 0), (INV, 1), (WOPEN, 1), (0x4003CF0C, 0), (0x400366A0, 0), (0x40034454, 0), (0x4003D6AC, 0),
    (0x400363CC, 0), (0x40036354, 0), (0x40036454, 0), (0x400362F4, 0), (0x400365F4, 0)],
    vals=(0, 1, 2, 3, 3, 3, 3, 3), extra=lambda r: compose(list_blk(r, 0x5DC6 - 20), writes([(SYS + 0xF9A, s16(r.choice([0, 1, 4, 6, 6, 0xFF])))]),
                                                           writes([(SYS + 0x84D6 - 4, bytes(r.choice([0, 0, 3])
                                                                                           for _ in range(48)))])))
TESTS[0x400525EC] = t_btn([0xFC, 0x17E, 0x192, 0x1A6, 0x1B0, 0x1BA, 0x1C4, 0x1CE, 0x1D8, 0x1E2, 0x1F6], [
    (0x4002F410, 0), (WOPEN, 1), (0x4003CF0C, 0), (0x40069018, 0), (0x4004A1D0, 0), (0x40043A80, 0)],
    vals=(0, 1, 2, 3, 3, 3, 3, 3, 3), extra=lambda r: rand_block(r, SYS + 0x658, 4))
TESTS[0x40055798] = t_btn([0x124, 0x12E, 0x138, 0x142, 0x14C], [
    (0x4002FD3C, 0), (0x4004435C, 0), (0x400443D4, 0), (0x40043EEC, 0), (0x40034134, 4), (0x40042CF0, 1),
    (0x40034284, 0), (WCLOSE, 1), (0x4003E708, 0), (0x4004D18C, 0), (0x4003CDC0, 0)], vals=(0, 1, 2, 3, 3),
    extra=lambda r: compose(win_rect(r, 0x76), msg_texts(r, 0xAAA0), rand_block(r, SYS + 0x7C0, 1),
                            writes([(0x42E64587, bytes([r.choice([0, 1, 2, 3, 4])]))])))
TESTS[0x40053940] = t_btn([0x5B6 + 10 * i for i in range(11)], [(0x4002F8E0, 0), (INV, 2), (WCLOSE, 1)],
                          vals=(0, 1, 2, 3, 3, 3, 3, 3, 3, 3), extra=lambda r: rand_block(r, SYS + 0x27A, 0x10))
TESTS[0x40056530] = t_btn([0xB92, 0xB9C, 0xBA6, 0xBB0, 0xBBA, 0xBC4, 0xBCE], [
    (0x4002FF48, 0), (0x4005F9F0, 2), (0x4005FA34, 2)], vals=(0, 1, 2, 3, 3, 3, 3),
    extra=lambda r: compose(rand_block(r, 0x42E68A88, 0x58), rand_block(r, 0x42E68668, 8),
                            rand_block(r, 0x42E68668 + 0x18C, 4)))


# ================================================================ batch 40: list A / C windows, settings buttons, self-tests
def list_extra(r):
    return compose(rand_block(r, SYS + 0xF00, 0x100), list_blk(r, 0x5DC6 - 20),
                   writes([(SYS + 0xF9A, s16(r.choice([0, 1, 4, 6, 6, 0xFF]))),
                           (SYS + 0x84D6 - 4, bytes(r.choice([0, 0, 3]) for _ in range(48)))]))


VB = (0, 1, 2, 3, 3, 3, 3, 3, 3, 3)
TESTS[0x40054C84] = t_btn([0x728 + 10 * i for i in range(7)] + [0x7BE + 10 * i for i in range(6)], [
    (0x4002FAAC, 0), (INV, 1), (WOPEN, 1), (0x4003CF0C, 0), (0x40035624, 0), (0x40034454, 0), (0x4003D6AC, 0),
    (0x40035320, 0), (0x400352A8, 0), (0x400353A8, 0), (0x40035248, 0), (0x40035578, 0)], vals=VB, extra=list_extra)
TESTS[0x400553A0] = t_btn([0x91C + 10 * i for i in range(7)] + [0x9B2 + 10 * i for i in range(6)], [
    (0x4002FC38, 0), (INV, 1), (WOPEN, 1), (0x4003CF0C, 0), (0x40034454, 0), (0x4003D6AC, 0), (0x40036C54, 0),
    (0x40036EA4, 1), (0x4004D12C, 0), (0x4004D18C, 0), (0x400377F8, 0), (0x4003777C, 0), (0x40037880, 0),
    (0x4003771C, 0)], vals=VB, extra=lambda r: compose(list_extra(r), rand_block(r, SYS + 0x3DA52C, 4)))
def one_btn(r, offs):
    """every button idle or pressed, then (most cases) one random button released"""
    w = [(BT35 + o, bytes([r.choice([0, 1, 3])])) for o in offs]
    if r.random() < 0.9:
        w.append((BT35 + r.choice(offs), bytes([2])))
    return writes(w)


def slider40(r):
    lo = r.choice([0, 1, 50, 300, 0xFFFF, r.getrandbits(16)])
    hi = (lo + r.choice([-1, 0, 1, 2, 7, 50, 100, 101, 1000, r.getrandbits(16)])) & 0xFFFF
    sl, sh = lo - (lo >> 15 << 16), hi - (hi >> 15 << 16)
    x = r.choice([sl - 1, sl, sl + 1, sh - 1, sh, sh + 1, 0, 0x7FFF, -0x8000, r.getrandbits(16)])
    return writes([(BT35 + 0x20B, bytes([r.choice([0, 0, 1, 1, 0x80])])), (BT35 + 0x20C, s16(lo)),
                   (BT35 + 0x210, s16(hi)), (0x42E68BB0, s16(x & 0xFFFF))])


TESTS[0x400527C0] = t_btn([0x214, 0x21E, 0x228, 0x17E, 0x232, 0x23C, 0x246], [
    (0x4002F490, 0), (WCLOSE, 1), (WOPEN, 1), (0x400367A4, 0), (0x4003F024, 0), (0x4003F034, 0), (0x4003EB40, 0),
    (0x4003ED28, 0), (0x4003CF0C, 0), (0x40034454, 0)], vals=VB,
    extra=lambda r: compose(rand_block(r, SYS + 0x650, 8), rand_block(r, SYS + 0x54E, 1),
                            one_btn(r, [0x214, 0x21E, 0x228, 0x17E, 0x232, 0x23C, 0x246]), slider40(r)))
TOP40 = [0]
TESTS[0x40052280] = t_btn([0x5C, 0x66, 0x70, 0x7A, 0x84, 0x8E, 0x98, 0xA2], [
    (0x4002F378, 0), (0x4003F0E0, 0), (0x4004C7C4, 0), (0x4003BE98, 2), (0x400424C4, 2), (0x4004A510, 0),
    (INV, 1), (0x4006382C, 0), (WOPEN, 1)], vals=VB,
    extra=lambda r: compose(TOP40.__setitem__(0, r.choice([-1, 0, 1, 4, 0xFE])) or rand_block(r, SYS + 0x3C, 8),
                            one_btn(r, [0x5C, 0x66, 0x70, 0x7A, 0x84, 0x8E, 0x98, 0xA2]),
                            rand_block(r, SYS + 0x100, 8),
                            rand_block(r, SYS + 0x660, 8), rand_block(r, SYS + 0x27E, 2), writes([
        (SYS + 0x2AA, bytes([r.choice([0, 1, 7])])), (SYS + 0x1BA, bytes([TOP40[0] + 1 & 0xFF])),
        (SYS + 0x41, bytes([TOP40[0] & 0xFF if r.random() < 0.5 else r.choice([0, 1, TOP40[0] - 1 & 0xFF, TOP40[0] + 1 & 0xFF, 0xFF])])),
        (SYS + 0x105, bytes([r.choice([0, 1, TOP40[0] - 1 & 0xFF, TOP40[0] & 0xFF, 0xFF])])),
        (SYS + 0x664, bytes([r.choice([0, 0, 0, 1, 9])])), (SYS + 0x665, bytes([r.choice([0, 1])]))])))
TESTS[0x40055CC4] = t_btn([0xA20 + 10 * i for i in range(16)], [(0x4002FDCC, 0), (0x4005FE94, 1), (INV, 1)],
                          vals=(0, 1, 2) + (3,) * 12)


def t_st40(qs):
    def t(r):
        import struct
        vals = [struct.pack("<q", r.choice(qs) * 1000 + r.choice([0, 1, 999, -1, -999]))
                if r.random() < 0.9 else struct.pack("<q", r.getrandbits(64) - (1 << 63)) for _ in range(16)]

        def acq(m, a):
            m.calls.append(("acq",))
            if m.vals:
                m.write(SYS + 0x8F8, m.vals.pop(0))
            if m.vals:
                m.write(SYS + 0x958, m.vals.pop(0))

        def out(m, a):          # the settings each step applies
            m.calls.append(("out", m.read(SYS + 0x3E, 6), m.read(SYS + 0x41, 1), m.r32(SYS + 0x60),
                            m.r32(SYS + 0x120), m.read(SYS + 0x588, 1), m.read(0x42E3F984 + 0x2EFA, 14)))

        def setup(m):
            m.vals = list(vals)
        return Case([], compose(rand_block(r, SYS + 0x8, 8), rand_block(r, 0x42E68AE0 + 0x20, 0x18), setup), ret=V,
                    stubs=(Stub("acq", 0x400419CC, acq, 0), rec("eb40", 0x4003EB40, 0), Stub("out", 0x4003E708, out, 0),
                           rec("gen", 0x4004D18C, 0), rec("delay", 0x40042CF0, 1)))
    return t


TESTS[0x400504B8] = t_st40([-401, -400, -399, -398, 0, 398, 399, 400, 401])
TESTS[0x40050688] = t_st40([-1501, -1500, -1499, -1498, 1498, 1499, 1500, 1501])


# ================================================================ batch 41: self-tests, touch test, dispatch, generator FPGA
def out41(m, a):            # the output settings each step applies
    m.calls.append(("out", m.read(SYS + 0x3E, 0x10), m.read(SYS + 0x102, 0x10), m.read(SYS + 0x54E, 2),
                    m.read(SYS + 0x588, 1), m.read(0x42E3F984 + 0x2EFA, 14), m.read(0x42E68BB0 + 0x16, 1)))


def t_spread(r):
    import random
    seed = r.getrandbits(32)
    spans = [r.choice([0, 1, 10, 50, 89, 90, 91, 100, 200, 255]) for _ in range(4)]
    end = r.choice([0, 0x27, 0x3B, 0x3C, 0x3D, 0x3E, 0x50, 0x100, 0x228])

    def acq(m, a):
        m.calls.append(("acq",))
        ph = (0 if m.read(SYS + 0x3E, 1)[0] else 2) + (m.read(SYS + 0x46, 1)[0] if m.read(SYS + 0x3E, 1)[0]
                                                         else m.read(SYS + 0x10A, 1)[0]) % 2
        sp = spans[ph]
        lo = m.rng.randint(0, 255 - sp)
        n = min(max(end - 0x28, 0), 0x200)
        m.write(0x42DF76D4, bytes(lo + m.rng.randint(0, sp) for _ in range(n)))

    def setup(m):
        m.rng = random.Random(seed)
    return Case([], compose(writes([(SYS + 0x640, s16(end))]), rand_block(r, 0x42E68AE0 + 0x38, 0x10), setup),
                ret=V, stubs=(Stub("acq", 0x400419CC, acq, 0), rec("eb40", 0x4003EB40, 0),
                              Stub("out", 0x4003E708, out41, 0)))


TESTS[0x40050BAC] = t_spread


def t_st41(qs, offs):
    def t(r):
        import struct
        vals = [struct.pack("<q", r.choice(qs) * 1000 + r.choice([0, 1, 999, -1, -999]))
                if r.random() < 0.9 else struct.pack("<q", r.getrandbits(64) - (1 << 63)) for _ in range(8)]

        def acq(m, a):
            m.calls.append(("acq",))
            for o in offs:
                if m.vals:
                    m.write(SYS + o, m.vals.pop(0))

        def setup(m):
            m.vals = list(vals)
        return Case([], compose(rand_block(r, 0x42E68AE0 + 0x38, 0x10), setup), ret=V,
                    stubs=(Stub("acq", 0x400419CC, acq, 0), rec("eb40", 0x4003EB40, 0), Stub("out", 0x4003E708, out41, 0),
                           rec("gen", 0x4004D18C, 0)))
    return t


TESTS[0x40050DDC] = t_st41([-801, -800, -1, 0, 1, 798, 799, 800, 801], (0x910, 0x970))


def t_touch41(r):
    centres = [(0x14, 0x14), (0x39C, 0x14), (0x14, 0x1F4), (0x39C, 0x1F4), (0x1D8, 0x104)]
    seq = []
    for _ in range(r.choice([0, 3, 10, 30])):
        cx, cy = r.choice(centres)
        seq.append((r.choice([0, 1, 2, 2, 2, 3]), cx + r.choice([0, 1, 2, 40, 79, 80, 81, -1]),
                    cy + r.choice([0, 1, 2, 40, 79, 80, 81, -1]) if r.random() < 0.8 else r.getrandbits(16)))
    r.shuffle(centres)
    seq += [(2, cx + 40, cy + 40) for cx, cy in centres]

    def poll(m, a):
        m.calls.append(("poll",))
        st, x, y = m.seq.pop(0) if m.seq else (2, 0x14 + 40, 0x14 + 40)
        m.w8(0x42E68BBC, st)
        m.write(0x42E68BB4, s16(x & 0xFFFF) + s16(y & 0xFFFF))

    def setup(m):
        m.seq = list(seq)
    return Case([], compose(rand_block(r, 0x42E68BB0, 0x10), rand_block(r, 0x42E68B20, 8), setup), ret=V,
                stubs=(Stub("poll", 0x400498E4, poll, 0),) + tuple(rec("%08x" % a, a, n) for a, n in (
                    (0x40049A38, 0), (0x400342DC, 0), (0x40034454, 0), (0x4002EA38, 1), (0x4002CCCC, 1),
                    (0x4002DC40, 1), (0x4002ECC4, 4), (0x4003442C, 0))))


TESTS[0x400519E4] = t_touch41
DISP41 = [0x40052100, 0x40052280, 0x400524B4, 0x400525D4, 0x400525EC, 0x40055720, 0x40055798, 0x40055AD4,
          0x400527C0, 0x400529F0, 0x40052F08, 0x40052FC4, 0x400531B0, 0x400533A0, 0x400537C4, 0x40053940,
          0x40053B78, 0x40054540, 0x40054C84, 0x40054EB8, 0x40054F08, 0x40054F50, 0x40055114, 0x40055344,
          0x400553A0, 0x400555E8, 0x40055634, 0x40055978, 0x40055B98, 0x40055BD4, 0x40055C78, 0x40055CC4,
          0x40055F2C, 0x40056080, 0x40056214, 0x400562F4, 0x40056404, 0x40056530, 0x40056724, 0x40056804, 0x40056974]
TESTS[0x40051E88] = lambda r: Case([r.choice(list(range(0, 45)) + [0xFF, 0x10001, 0xFFFFFFFF, r.getrandbits(32)])],
                                   ret=V, stubs=(rec("wtouch", 0x4002F228, 1),) +
                                   tuple(rec("%08x" % a, a, 0) for a in DISP41))


def t_genfpga(r):
    G = 0x42E3F984
    return Case([], compose(rand_block(r, G + 0xFB6, 0x60), rand_block(r, G + 0x2EF0, 0x80), writes([
        (0x42E68BB0 + 0x17, bytes([r.choice([0, 0, 0, 0, 1, 0x80])])),
        (G + 0x2EF6, s16(r.choice([0, 1, 2, 3, 5, 0x10, 0x2F, 0x101]))),
        (G + 0x2EFA, bytes([r.choice([0, 1, 0x80])])),
        (G + 0x2EFC, i32(r.choice([0, 1, 100, 249, 250, 500, 2499, 2500, 0x80000000, r.getrandbits(32)]))),
        (G + 0x2F5C, i32(r.choice([0, 1, 249, 250, 251, 2499, 2500, 2501, 0xFFFFFFFF, r.getrandbits(32)])))])),
        ret=V, stubs=(rec("sel", SEL, 1), rec("wr", WR, 1), rec("clk", 0x4005F38C, 1), rec("fill", 0x4004EC98, 1)))


TESTS[0x4004ED18] = t_genfpga


# ================================================================ batch 42: self-tests, numpad parser, measurement limits
def out42(m, a):            # output settings and offsets each step applies
    m.calls.append(("out", m.read(SYS + 0x3A, 0x10), m.read(SYS + 0x102, 0x10), m.read(SYS + 0x588, 1),
                    m.r32(SYS + 0x60), m.r32(SYS + 0x120), m.read(SYS + 0x660, 1), m.read(0x42E68BB0 + 0x16, 1)))


def t_st4f89c(r):
    import random
    seed = r.getrandbits(32)
    # per phase (acquisition pair): word that must be above e * 1.3, word that must be below e * 0.7
    need = [(0xB34, 0xB50), (0xB50, 0xB34), (0xB34, None), (None, 0xB50)]
    slots = [(ph, o, kind) for ph, (hi, lo) in enumerate(need) for o, kind in ((hi, "hi"), (lo, "lo")) if o]
    edge = r.choice(slots + [None]) if r.random() < 0.7 else "rand"

    def acq(m, a):
        m.calls.append(("acq",))
        m.nacq = getattr(m, "nacq", 0) + 1
        ph = min((m.nacq - 1) // 2, 3)
        e = m.read(SYS + 0xE, 2)
        e = e[0] | e[1] << 8
        for o in (0xB34, 0xB50):
            kind = next((k for p_, oo, k in slots if p_ == ph and oo == o), None)
            if edge == "rand":
                c = m.rng.choice([0, int(e * 1.3), int(e * 1.3) + 1, int(e * 0.7), int(e * 0.7) - 1, e,
                                  0xFFFFFFFF, m.rng.getrandbits(32)])
            elif kind is None:
                c = m.rng.choice([0, e, int(e * 1.3) + 1, 0xFFFFFFFF])
            elif edge == (ph, o, kind):
                c = int(e * 1.3) if kind == "hi" else int(e * 0.7)
            else:
                c = int(e * 1.3) + 1 if kind == "hi" else max(int(e * 0.7) - 1, 0)
            m.w32(SYS + o, c & 0xFFFFFFFF)

    def setup(m):
        m.rng = random.Random(seed)
    return Case([], compose(writes([(SYS + 0xE, s16(r.choice([0, 1, 7, 100, 100, 100, 1000, 3000, 0xFFFF]))),
                                    (SYS + 0x20, s16(r.choice([0, 1, 50, 0x8000, r.getrandbits(16)])))]),
                            rand_block(r, 0x42E68AE0 + 0x10, 8), setup), ret=V,
                stubs=(Stub("acq", 0x400419CC, acq, 0), rec("eb40", 0x4003EB40, 0), Stub("out", 0x4003E708, out42, 0)))


TESTS[0x4004F89C] = t_st4f89c


def t_lanes(r):
    import random
    seed = r.getrandbits(32)
    modes = [r.choice(["rand", "rand", "const", "two", "walk", "noedge", "first", "first", "last"]) for _ in range(8)]

    def cap(m, a):
        m.calls.append(("cap",))
        m.ncap = getattr(m, "ncap", 0) + 1
        for k in range(8):
            md = modes[k]
            if md in ("first", "last"):
                v = 0xAA if m.ncap - 1 == (10 if md == "first" else 889) else 0x55
            elif md == "rand":
                v = m.rng.getrandbits(8)
            elif md == "const":
                v = 0x5A
            elif md == "two":
                v = m.rng.choice([0x00, 0xFF])
            elif md == "walk":
                v = 1 << m.rng.randrange(8)
            else:
                v = m.rng.choice([0x00, 0x0F, 0xF0, 0xFF, 0x03, 0xFC])
            m.w8(0x42DF76D4 + k * 0x2134, v)

    def sel2a(m, a):
        m.calls.append(("2a", m.read(SYS + 0x44, 2)))

    def setup(m):
        m.rng = random.Random(seed)
    return Case([], compose(rand_block(r, 0x42E68AE0 + 0x10, 0x18), setup), ret=V,
                stubs=(Stub("cap", 0x400406DC, cap, 0), Stub("2a", 0x4004AA94, sel2a, 0), rec("eb40", 0x4003EB40, 0),
                       Stub("out", 0x4003E708, out42, 0), rec("geom", 0x400405E0, 0), rec("delay", 0x40042CF0, 1),
                       rec("prep", 0x4003FA10, 0), rec("trig", 0x4003FAB8, 0)))


TESTS[0x4004F5E4] = t_lanes


def t_npparse(r):
    import struct
    txt = ""
    if r.random() < 0.3:
        txt += "-"
    if r.random() < 0.3:                      # long numbers: beyond the int32 range
        txt += "".join(r.choice("0123456789") for _ in range(r.choice([9, 10, 11, 14])))
    for _ in range(r.choice([0, 1, 3, 9, 19, 22])):
        txt += r.choice("0123456789" * 3 + ".x:/")
    b = txt.encode()[:23] + bytes([r.choice([0, 0, 0x20, 0x80])])
    out = SYS + 0x200000
    return Case([r.choice([0xE, 0xE, 0xF, 0xF, 0x10, 0x10, 0, 0x11])], compose(
        rand_block(r, 0x42E68A88, 0x58), rand_block(r, out, 0x20),
        writes([(0x42E68A89, bytes([r.choice([0, 1, 2, 3, 4, 5, 6, 7])])), (0x42E68A8B, b + bytes(24 - len(b))),
                (0x42E68AA8, struct.pack("<d", r.choice([0.0, -1.5, 1e300, r.uniform(-1e9, 1e9)]))),
                (0x42E68AB0, i32(out) + i32(out + 8) + i32(out + 0x10)),
                (0x42E68AD4, bytes([r.choice([0, 1])])), (0x42E68AD8, i32(0x400617AC))])), ret=V,
        stubs=(rec("cb", 0x400617AC, 0),))


TESTS[0x4005FC84] = t_npparse


def t_mclamp(r):
    import struct
    big = [0, 1, -1, 999999999, 1000000000, 1000000001, 1 << 40, -(1 << 40), (1 << 63) - 1, -(1 << 63)]

    def pick():
        return r.choice(big) if r.random() < 0.8 else r.getrandbits(64) - (1 << 63)
    after = [pick() for _ in range(4)]

    def recalc(m, a):
        m.calls.append(("recalc", m.read(0x42E68668 + 0x120, 0x20)))
        for i, v in enumerate(after):
            if v % 3 == 0:
                m.write(0x42E68668 + 0x120 + 8 * i, struct.pack("<q", v))
    dbl = [260.0, 260.00000000000006, 260.0000000000001, 259.99, -60.0, -60.000000001, 1.0, 0.999999, 0.0,
           float("nan"), float("inf"), -float("inf"), 100.0, -59.5, 0.5]
    return Case([], compose(writes([(0x42E68668 + 0x120, b"".join(struct.pack("<q", pick()) for _ in range(4))),
                                    (0x42E68668 + 0x140, struct.pack("<d", r.choice(dbl)) + struct.pack("<d", r.choice(dbl))),
                                    (0x42E68BC7, bytes([r.choice([0, 0, 0, 1])]))])), ret=V,
                stubs=(Stub("recalc", 0x40061680, recalc, 0),))


TESTS[0x400617AC] = t_mclamp


# ================================================================ batch 43: trace segment scaling, level columns
def t_tscale(r):
    tr = r.choice([0, 1, 2, 3])
    start = r.choice([0, 1, 5, 40])
    ln = r.choice([0, 1, 7, 30, 0xFFFFFFFF, 0xFFFFFFFE])
    sel = r.choice([0, 1, 1, 2])
    m01 = r.choice([0, 1])
    k = r.choice([1, 1, 2, 16, 0xFFFFFFFF])
    w = lambda v: i32(v)
    return Case([tr, start, ln], compose(
        rand_block(r, 0x42AC7CF4 + sel * 0x62250, 0x80),
        writes([(SYS + 0x1C0, w(k)), (SYS + 0xA, s16(r.choice([0, 1, 5, 6, 480, 600, 0xFFFF]))),
                (SYS + 0x2AA, bytes([r.choice([0, 0, 1, 3])])),
                (SYS + 0xE4, w(r.choice([0, 1, 1024, 2000, -1024, r.getrandbits(32)]))),
                (SYS + 0x1A4, w(r.choice([0, 1, 1024, 3000, -500, r.getrandbits(32)]))),
                (SYS + 0xE8, w(r.choice([0, 1, 100, 0x8000, -100]))), (SYS + 0x1A8, w(r.choice([0, 1, 300, -7]))),
                (SYS + 0x60, w(r.choice([0, 1, 200, 0x800, -3]))), (SYS + 0x120, w(r.choice([0, 1, 250, 0x900, -9])))])),
        ret=V, stubs=(rec("m02", 0x4003BF58, 1, sel), rec("m01", 0x4003BF78, 1, m01)))


TESTS[0x4005F778] = t_tscale


def colour_stub(m, a):
    m.calls.append(("colour", a[0]))
    m.write(0x42E68660, s16(a[0] & 0xFFFF))


def t_colsb(r):
    M = 0x42E68668
    n = r.choice([0, 1, 3, 5])
    h = r.choice([0, 1, 4, 12, 20])
    m174 = r.choice([0, 1, 3, h - 1, h + 2]) & 0xFFFFFFFF
    if (h - m174) & 0xFFFFFFFF == 0:
        m174 = (m174 + 1) & 0xFFFFFFFF
    scr = SYS + 0x200000
    return Case([], compose(
        rand_block(r, 0x42E583A0, 0x400),
        writes([(0x42E68BC7, bytes([r.choice([0, 0, 0, 1])])), (M + 2, bytes([r.choice([0, 1, 1, 1])])),
                (M + 0xEC, i32(n)), (M + 0xF0, i32(h)), (M + 0x174, i32(m174)),
                (M + 0xE8, i32(r.choice([0, 1, 3]))), (M + 0xE4, i32(r.choice([0, 2, 9]))), (0x40095498, i32(scr)),
                (0x42E61000, b"".join(s16(r.choice([0, 1, 3, h, h + 1, max(h - 2, 0)])) for _ in range(6)))])),
        ret=V, stubs=(Stub("colour", 0x4002EA38, colour_stub, 1), rec("copy", 0x40035728, 3)))


TESTS[0x40061194] = t_colsb


def t_cols(r):
    M = 0x42E68668
    n = r.choice([0, 1, 3, 5])
    h = r.choice([1, 2, 4, 12, 20])
    scr = SYS + 0x200000
    return Case([], compose(
        rand_block(r, 0x42E56C30, 0x400), rand_block(r, 0x42B8C194, 0x100),
        writes([(0x42E68BC7, bytes([r.choice([0, 0, 0, 1])])), (M + 1, bytes([r.choice([0, 1, 1, 1])])),
                (M + 0xDC, i32(n)), (M + 0xE0, i32(h)), (M + 0x184, i32(r.choice([0, 1, 3, h, -1]))),
                (M + 0xD8, i32(r.choice([0, 1, 3]))), (M + 0xD4, i32(r.choice([0, 2, 9]))), (0x40095498, i32(scr)),
                (0x42E60008, b"".join(s16(r.choice([0, 1, 3, h, h + 1, max(h - 2, 0), 0xFFFE])) for _ in range(6)))])),
        ret=V, stubs=(Stub("colour", 0x4002EA38, colour_stub, 1), rec("copy", 0x40035728, 3)))


TESTS[0x40062C98] = t_cols


def t_autolvl(r):
    import random
    seed = r.getrandbits(32)
    h = r.choice([0, 1, 10, 100, 480, 0xFFFF])

    def acq(m, a):
        m.calls.append(("acq", m.read(SYS + 0x41, 1)))
        hi, lo = int(h * 0.9), int(h * 0.2)
        m.w32(SYS + 0xB30, m.rng.choice([0, lo - 1, lo, lo + 1, hi - 1, hi, hi + 1, 0xFFFFFFFF]) & 0xFFFFFFFF)

    def setup(m):
        m.rng = random.Random(seed)
    M = 0x42E68668
    return Case([], compose(rand_block(r, SYS + 0x910, 8), writes([
        (0x42E68BC7, bytes([r.choice([0, 0, 0, 1])])), (SYS + 0xA, s16(h)),
        (SYS + 0x41, bytes([r.choice([0, 1, 3, 6, 7, 8, 0xFF])])),
        (M + 1, bytes([r.choice([0, 1])])), (M + 2, bytes([r.choice([0, 1])])),
        (M + 0xDC, i32(r.choice([0, 1, 4]))), (M + 0xE0, i32(r.choice([0, 1, 5, 9]))),
        (M + 0xEC, i32(r.choice([0, 1, 3]))), (M + 0xF0, i32(r.choice([0, 2, 6]))),
        (M + 0x184, i32(r.choice([0, 1, 2, 7, -1])))]), setup), ret=V,
        stubs=(Stub("acq", 0x400419CC, acq, 0), Stub("out", 0x4003E708, out42, 0), rec("fftb", 0x40061580, 0),
               rec("copy", 0x40035728, 3)))


TESTS[0x40060FE8] = t_autolvl


# ================================================================ batch 44: text and numbers
STR44 = SYS + 0x210000
MAPPED44 = [0x20, 0x21, 0x25, 0x28, 0x2A, 0x2F, 0x3A, 0x3D, 0x3F, 0x5F, 0xC4, 0xD1, 0xDF, 0xE0, 0xE9, 0xF1, 0xFC,
            0x101, 0x113, 0x14D, 0x1CE, 0x1DC, 0x261, 0x401, 0x410, 0x416, 0x42F, 0x430, 0x44F, 0x451, 0x1E9E,
            0xFF08, 0xFF09]


def str44(r):
    toks = []
    for _ in range(r.choice([0, 1, 2, 4, 7])):
        k = r.choice("aaaddpuu2233xq")
        if k == "a":
            toks.append(bytes([r.choice(b"AZaz09Mq.+-# ~!%")]))
        elif k == "d":
            toks.append(bytes([r.randrange(0x30, 0x3A)]))
        elif k == "p":
            toks.append(bytes([r.choice(b".+-")]))
        elif k == "u":
            toks.append(chr(r.choice(MAPPED44)).encode())
        elif k == "2":
            toks.append(chr(r.choice([0xC4, 0xE9, 0x416, 0x44F, 0x7FF, 0x100, r.randrange(0x80, 0x800)])).encode())
        elif k == "3":
            toks.append(chr(r.choice([0x1E9E, 0xFF08, 0x20AC, 0x4E2D, r.randrange(0x800, 0xD800)])).encode())
        elif k == "x":
            toks.append(chr(r.choice([0x1F600, 0x10401, r.randrange(0x10000, 0x110000)])).encode())
        else:
            toks.append(bytes([r.choice([0x80, 0xBF, 0xF8, 0xFF, 0xC3, 0xE2])]))
    b = b"".join(toks)
    if b and (b[0] & 0xC0) == 0x80 or b[:1] in (b"\xf8", b"\xff"):
        b = b"A" + b
    return b + bytes(64)


def font44(r, t):
    return [(FD, bytes([t]) + bytes(r.getrandbits(8) for _ in range(0x0F))),
            (FD + 8, s16(r.choice([0, 1, 9, 14]))),
            (FD + 0x262, b"".join(s16(r.choice([0, 1, 5, 8, 12, 0xFFFF])) for _ in range(140))),
            (0x42E68C74, i32(FD)), (0x42E68BE2, s16(r.choice([0, 1, 2, 0xFFFF]))),
            (0x42E68BE0, bytes([r.choice([0, 0, 1, 0x80])]))]


def t_twidth(r):
    t = r.choice([1, 1, 1, 8, 8, 0, 3])
    return Case([STR44], writes(font44(r, t) + [(STR44, str44(r))]), ret=R)


TESTS[0x4002D164] = t_twidth


def t_tdraw(r):
    t = r.choice([1, 1, 1, 8, 8, 0, 3])
    return Case([xy(r), xy(r), STR44], compose(rand_block(r, 0x42CF64B4, 0x1000),
                                              writes(font44(r, t) + [(STR44, str44(r))])), ret=V,
                stubs=(rec("chr", 0x4002E6D8, 3), rec("tint", 0x4002E878, 3), rec("g14", 0x4002CDC4, 3)))


TESTS[0x4002CE7C] = t_tdraw


def val44(r):
    v = r.choice([0, 1, 9, 10, 99, 100, 12345, 987654321, 10 ** 15 - 1, 10 ** 15, 10 ** 16 + 7, 10 ** 18 * 9,
                  2 ** 63 - 1, r.getrandbits(20), r.getrandbits(40), r.getrandbits(63)])
    if r.random() < 0.4:
        v = -v
    if r.random() < 0.05:
        v = -2 ** 63
    return v & 0xFFFFFFFFFFFFFFFF


def str_stub(name, addr, ret=None, si=0):
    """Records the string argument si by content (the buffer is on the caller's frame) and the
    arguments before it."""
    def fn(m, a):
        a = regs(m, si + 1)
        b = m.read(a[si], 40)
        m.calls.append((name, b[:b.index(0) if 0 in b else 40]) + tuple(a[:si]))
        return ret
    return Stub(name, addr, fn, 0)


def t_numw(r):
    v = val44(r)
    return Case([v & 0xFFFFFFFF, v >> 32, r.choice([0, 0, 1, 2, 3, 6, 15]),
                 r.choice([0, 0, 1, 1, 0xFFFFFFFF, 2, 0x101])], ret=R,
                stubs=(str_stub("tw", 0x4002D164, r.getrandbits(32)),))


TESTS[0x4002D2D4] = t_numw


def t_numd(r):
    v = val44(r)
    dec = (r.getrandbits(24) << 8) | r.choice([0, 0, 1, 2, 3, 6, 15])
    mode = (r.getrandbits(24) << 8) | r.choice([0, 0, 1, 1, 0xFF, 2, 0x81])
    return Case([xy(r), xy(r), v & 0xFFFFFFFF, v >> 32], ret=V, stubs=(str_stub("td", 0x4002CE7C, None, 2),),
                stack=[dec, mode])


TESTS[0x4002D82C] = t_numd


def t_gidx(r):
    c = r.choice(MAPPED44 + [0x30, 0x39, 0x3A, 0x40, 0x41, 0x5A, 0x5B, 0x60, 0x61, 0x7A, 0x7B, r.randrange(0x20, 0x80), r.randrange(0x20, 0x80), r.randrange(0x400, 0x460),
                             r.randrange(0xC0, 0x200), r.getrandbits(16), r.getrandbits(32)])
    if r.random() < 0.1:
        c |= r.choice([0x10000, 0x80000000, 0xFFFF0000])
    return Case([c], ret=R)


TESTS[0x4002DD70] = t_gidx


def t_galpha(r):
    fb = SYS + 0x100000
    w, h = r.choice([0, 1, 3, 6]), r.choice([0, 1, 2, 4])
    x, y = r.choice([0, 3, 17, 1000]), r.choice([0, 1, 5])
    g = bytes([w >> 8, w, h >> 8, h]) + bytes(r.choice([0, 1, 2, 3, 4, 100, 200, 0xF5, 0xF6, 0xFF]) for _ in range(w * h))
    px = [(fb + 2 * (x + (y + j) * 1022), b"".join(s16(r.choice([0, 0xFFFF, 0x0821, r.getrandbits(16)]))
                                                    for _ in range(w))) for j in range(h)]
    col = bytes([r.choice([0, 0, 0xFF, 1, r.getrandbits(8)])]) + bytes(r.getrandbits(8) for _ in range(3)) + \
        s16(r.getrandbits(16))
    return Case([x, y, GD], writes([(GD, g), (0x40095498, i32(fb)), (0x42E6865C, col),
                                    (0x42E68BE6, s16(r.choice([0, 3, 50, 0xFFFF])))] + px), ret=V)


TESTS[0x4002EE78] = t_galpha


# ================================================================ batch 45: SD update check, flash read / RMW
SPIBUF45 = SYS + 0x220000
SRC45 = SYS + 0x230000
SECTOR45 = 0x42E5E010
IMAGE45 = 0x41E7B9C8


def t_pread(r):
    dev = r.choice([0, 1, 2, r.getrandbits(32)])
    addr = r.choice([0, 0xA8000, r.getrandbits(24), r.getrandbits(32)])
    n = r.choice([0, 1, 4, 16, 64])
    four = r.choice([0, 0, 1, r.getrandbits(32)])
    mode = r.choice([0, 0, 1, 1, 2, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF])
    data = bytes(r.getrandbits(8) for _ in range(n))
    res = r.choice([0, 0, 0, 0xFFFFFFFF, 1, 0x80000000])

    def wtr(m, a):
        a = regs(m, 4)
        nrx = m.r32(m.reg(A.UC_ARM_REG_SP))
        m.calls.append(("wtr", a[0], m.read(a[1], a[2] & 0xFF) if a[2] < 16 else a[2], a[2], a[3], nrx))
        m.write(a[3], data[:nrx])
        return res

    def setup(m):
        m.w32(0x42E68CD4, four)
        m.w32(0x42E68CC4, mode)
    return Case([dev, addr, SPIBUF45, n], setup=setup, ret=R,
                stubs=(rec("sel", 0x4004302C, 1), rec("desel", 0x40043088, 1), Stub("wtr", 0x40043360, wtr, 0)))


TESTS[0x40043620] = t_pread


def t_prmw(r):
    dev = r.choice([0, 1, r.getrandbits(32)])
    off = r.choice([0, 0, 1, 0xFFF, 0x800, r.getrandbits(12)])
    addr = (r.getrandbits(r.choice([8, 12, 20])) << 12) | off
    n = r.choice([0, 1, 2, 0x10, 0x1000 - off, 0x1001 - off, 0x1000, 0x1001, 0x2000, r.randrange(1, 0x3000)])
    src = bytes(r.getrandbits(8) for _ in range(n))
    blocks = []
    for _ in range(5):
        k = r.choice("ffffxr")
        b = bytearray(b"\xff" * 0x1000)
        if k == "x":
            for _ in range(r.choice([1, 1, 3])):
                b[r.randrange(0x1000)] = r.getrandbits(8)
        elif k == "r":
            b = bytearray(r.getrandbits(8) for _ in range(0x1000))
        blocks.append(bytes(b))

    def pread(m, a):
        m.calls.append(("pread",) + tuple(a))
        m.write(a[2], blocks[m.nread % len(blocks)])
        m.nread += 1
        return 0

    def setup(m):
        m.nread = 0
        m.write(SRC45, src)
        m.write(SECTOR45, bytes(r2 & 0xFF for r2 in range(0x1000)))
    return Case([dev, addr, SRC45, n], setup=setup, ret=0,
                stubs=(Stub("pread", 0x40043620, pread, 4), rec("erase", 0x40043564, 2),
                       rec("wpaged", 0x4004380C, 4)))


TESTS[0x40043874] = t_prmw


def chk45(data):
    """The nine big-endian checksums sd_update_check expects after data."""
    s = [0] * 9
    for i, c in enumerate(data):
        s[0] += c
        for k in range(8):
            if c & (0x80 >> k):
                s[1 + k] += i
    return b"".join((v & 0xFFFFFFFF).to_bytes(4, "big") for v in s)


def name45(r):
    return r.choice([b"fw.iso", b"fw.iso", b"DPOS350P.iso", b"DPOS350P.iso", b"a.iso", b"a.iso", b"x.ISO", b".iso", b"a.b.iso", b"a.isox", b"ab.is",
                     b"readme.txt", b"noext", b"iso", b"q.iso", b"x" * 250 + b".iso", b"x" * 300 + b".iso"])[:255]


def t_sdup(r):
    ents = []
    for _ in range(r.choice([0, 1, 1, 2, 3, 5])):
        nm = name45(r)
        size = r.choice([0, 999, 0x1024, 0x1025, r.randrange(0x1025, 0x1400)]) | (r.choice([0, 0, 1]) << 32)
        ents.append((nm, size, r.choice([0, 0, 0, 0, 1])))
    if r.random() < 0.6:
        ents.insert(r.randrange(len(ents) + 1), (r.choice([b"fw.iso", b"DPOS350P.iso"]),
                                                 r.choice([0x1024, r.randrange(0x1025, 0x1400)]), 0))
    if r.random() < 0.7:
        ents.append((b"", 0, 0))
    hdr = bytearray(r.getrandbits(8) for _ in range(0x1000))
    hdr[:8] = b"DPOS350P"
    if r.random() < 0.2:
        hdr[r.randrange(9)] ^= 1 << r.randrange(8)
    sizes = [e[1] & 0xFFFFFFFF for e in ents if e[1] & 0xFFFFFFFF >= 0x1024]
    n = (r.choice(sizes) if sizes else 0x1100) - 0x1000
    body = bytes(r.choice([0, 0xFF, r.getrandbits(8)]) for _ in range(n - 36))
    tail = bytearray(chk45(body))
    if r.random() < 0.3:
        tail[r.randrange(36)] ^= 1 << r.randrange(8)
    payload = body + bytes(tail)
    open_ret = r.choice([0, 0, 0, 4])
    dir_ret = r.choice([0, 0, 5])

    def opendir(m, a):
        a = regs(m, 2)
        m.calls.append(("opendir", m.read(a[1], 3)))
        return dir_ret

    def readdir(m, a):
        a = regs(m, 2)
        m.calls.append(("readdir",))
        if m.closed:
            return 9
        if not m.ents:
            return 1
        nm, size, ret = m.ents.pop(0)
        m.write(a[1], size.to_bytes(8, "little"))
        m.write(a[1] + 0x1A, nm + b"\0")
        return ret

    def closedir(m, a):
        m.closed = True
        m.calls.append(("closedir",))

    def path(m, p):
        b = m.read(p, 256)
        return b[:b.index(0)] if 0 in b else b

    def fopen(m, a):
        a = regs(m, 3)
        m.calls.append(("open", path(m, a[1]), a[2] & 0xFF))
        return open_ret

    def fread(m, a):
        a = regs(m, 4)
        m.calls.append(("read", a[1], a[2], a[3]))
        d = (bytes(hdr), payload)[min(m.nread, 1)]
        m.nread += 1
        m.write(a[1], d[:a[2]])
        return 0

    def lseek(m, a):
        a = regs(m, 4)
        m.calls.append(("lseek", a[2], a[3]))
        return 0

    def unlink(m, a):
        a = regs(m, 1)
        m.calls.append(("unlink", path(m, a[0])))
        return 0

    def setup(m):
        m.ents = list(ents)
        m.closed = False
        m.nread = 0
    return Case([], setup=setup, ret=0,
                stubs=(Stub("opendir", 0x4005AA2C, opendir, 0), Stub("readdir", 0x4005AB48, readdir, 0),
                       Stub("closedir", 0x4005AB20, closedir, 0), Stub("open", 0x40059B8C, fopen, 0),
                       Stub("read", 0x40059F7C, fread, 0), Stub("lseek", 0x4005A6CC, lseek, 0),
                       rec("close", 0x4005A694, 0), Stub("unlink", 0x4005AD8C, unlink, 0),
                       rec("memclr", 0x40068E5C, 2), rec("rmw", 0x40043874, 4), rec("reset", 0x40067574, 0)))


TESTS[0x4004F134] = t_sdup


# ================================================================ batch 46: dialogs, colour buttons, XY grid, markers
STR46 = SYS + 0x250000
TXT46 = 0x42C2FE74
BTN46 = 0x42E61FF8
BOFF46 = [0x156, 0x160, 0x16A, 0x174, 0x19C, 0x7FA, 0x188, 0x1EC, 0x200, 0x912, 0x9EE, 0x9F8, 0xA02, 0xA0C, 0xA16, 0xB4C, 0xC28]


def ui46(r):
    """Random window rectangles, colour tables, UI text entries 90..130, button bytes."""
    rects = bytes(r.getrandbits(8) for _ in range(0x120))
    cols = bytes(r.getrandbits(8) for _ in range(0x40))
    ents = []
    for k in [11, 67] + list(range(90, 131)):
        txt = bytes(r.choice(b"abcXYZ !?.") for _ in range(r.randrange(0, 12)))
        ents.append((k, txt, r.getrandbits(8), r.getrandbits(8)))
    btn = {o: r.choice([0, 0, 1, 1, 2, 0xFF]) for o in BOFF46}
    v88 = r.getrandbits(16)
    lang = r.choice([0, 1, 2, 3, 4, 5])
    be4 = r.getrandbits(16)

    def setup(m):
        m.write(0x42E64520 + 0x150, rects)
        m.write(SYS + 0x3DD530, cols)
        for k, txt, dx, dy in ents:
            e = TXT46 + 0x1A0 * k
            sp = STR46 + 0x20 * k
            m.write(sp, txt + b"\0")
            m.w32(e, sp)
            m.write(e + 0x194, bytes([dx, dy]))
        for o, v in btn.items():
            m.write(BTN46 + o + 1, bytes([v]))
        m.write(SYS + 0x88BE, v88.to_bytes(2, "little"))
        m.write(0x42E68BE4, be4.to_bytes(2, "little"))
        m.write(SYS + 0x7AC, bytes([lang]))
    return setup


def tsd46(r):
    seq = [r.getrandbits(16) for _ in range(8)]

    def fn(m, a):
        a = regs(m, 4)
        b = m.read(a[2], 40)
        m.calls.append(("tsd", a[0], a[1], b[:b.index(0) if 0 in b else 40], a[3]))
        m.write(0x42E68BE4, seq[m.ntsd % 8].to_bytes(2, "little"))
        m.ntsd += 1
    return Stub("tsd", 0x4002D0D4, fn, 0)


def ui_stubs46(r):
    return (rec("colour", 0x4002EA38, 1), rec("rect", 0x4002ECC4, 4), rec("half", 0x4002DC40, 1),
            rec("font", 0x4002CCCC, 1), str_stub("tw", 0x4002D164, r.getrandbits(r.choice([4, 8, 16, 32]))),
            tsd46(r), rec_stk("touch", 0x4002F160, 1), rec_stk("d6f8", 0x4002D724, 4),
            rec_stk("dts", 0x4002F0E8, 2), rec("fbclr", 0x40034454, 0))


def t_dlg46(r):
    st = ui46(r)

    def setup(m):
        m.ntsd = 0
        st(m)
    return Case([], setup=setup, ret=0, stubs=ui_stubs46(r))


for a in (0x40024E38, 0x40029BC0, 0x400292E4, 0x400262F8, 0x400271B0, 0x4002903C, 0x4002C32C, 0x4002967C, 0x40029920, 0x40029EC0, 0x40028084):
    TESTS[a] = t_dlg46


def geom46(r):
    g = r.getrandbits
    f = {4: (2, [0, 10, 100, 500, g(16)]), 6: (2, [0, 5, 50, g(16)]), 8: (2, [0, 400, 800, 1000, g(16)]),
         0xA: (2, [0, 1, 7, 400, 504, g(16)]), 0x20: (2, [1, 2, 25, 50, 63, 200]), 0x655: (1, [0, 1, 0x80]),
         0x60: (4, [0, 5, 100, 0xFFFFFFF0, g(32)]), 0x120: (4, [0, 3, 100, 250, 0xFFFFFF00, g(32)])}
    return sys_fields(r, f)


def t_xy46(r):
    return Case([], setup=geom46(r), ret=0,
                stubs=(rec("colour", 0x4002EA38, 1), rec("span", 0x4002EBA8, 3), rec("vline", 0x4002EBC8, 3)))


TESTS[0x4002C5F8] = t_xy46


def t_mark46(r):
    return Case([], setup=geom46(r), ret=0,
                stubs=(rec("colour", 0x4002EA38, 1), rec("rect", 0x4002ECC4, 4), rec("font", 0x4002CCCC, 1),
                       str_stub("text", 0x4002CE7C, si=2), rec("span", 0x4002EBA8, 3),
                       rec("vline", 0x4002EBC8, 3)))


TESTS[0x40024C60] = t_mark46


def t_cur46(r):
    g = r.getrandbits
    st = geom46(r)
    en = r.choice([0, 1, 1, 1, 0x80])
    ch = r.choice([0, 1, 2])
    c1 = r.choice([0, 10, 300, g(10), g(32)])
    c2 = r.choice([0, 70, 400, c1 + 60, c1 + 61, g(10), g(32)])

    def setup(m):
        st(m)
        m.write(SYS + 0x3DA52C, bytes([en]))
        m.write(SYS + 0x3DA530, bytes([ch]))
        m.w32(SYS + 0x3DA534, c1)
        m.w32(SYS + 0x3DA538, c2)
    return Case([], setup=setup, ret=0,
                stubs=(rec("colour", 0x4002EA38, 1), rec("rect", 0x4002ECC4, 4), rec("font", 0x4002CCCC, 1),
                       rec("half", 0x4002DC40, 1), str_stub("text", 0x4002CE7C, si=2),
                       str_stub("tw", 0x4002D164, g(r.choice([4, 8, 32]))), rec("span", 0x4002EBA8, 3),
                       rec("vline", 0x4002EBC8, 3)))


TESTS[0x40028364] = t_cur46


# ================================================================ batch 47: measure_channel
BUF47 = SYS + 0x260000
import math as _m47


def wave47(r, n, banks):
    kind = r.choice("sssqqqttrrnc")
    per = r.choice([4, 7, 16, 33, 64, 100, 250]) * banks
    lo = r.choice([0, 100, 1000, 20000])
    amp = r.choice([0, 1, 50, 500, 500, 4000, 4000, 30000, 30000])
    duty = r.choice([0.1, 0.5, 0.5, 0.8])
    ph = r.randrange(per)
    out = []
    for i in range(n):
        x = ((i + ph) % per) / per
        if kind == "s":
            v = lo + (amp if x < duty else 0)
        elif kind == "q":
            v = lo + amp * (0.5 + 0.5 * _m47.sin(2 * _m47.pi * x))
        elif kind == "t":
            v = lo + amp * (2 * x if x < 0.5 else 2 - 2 * x)
        elif kind == "n":
            v = lo + r.randrange(amp + 1)
        elif kind == "c":
            v = lo
        else:
            v = lo + (amp if x < duty else 0) + r.randrange(amp // 8 + 1)
        out.append(int(v) & 0xFFFF)
    return out


def t_meas47(r):
    g = r.getrandbits
    ch = r.choice([0, 0, 0, 1, 1, 1]) if r.random() < 0.95 else r.choice([0x100, 0x101])
    n = r.choice([20, 21, 23, 30, 64, 100, 300, 300, 512, 512, 700, 700, 700])
    banks = r.choice([1, 1, 1, 2, 2, 3, 4]) if r.random() < 0.97 else r.choice([0, 0xFFFFFFFF])
    samples = wave47(r, n + 16, max(banks if banks < 100 else 1, 1))
    data = b"".join(v.to_bytes(2, "little") for v in samples)
    sh = r.choice([0, 0, 1, 3])
    words = {
        0x3E: (1, r.choice([1, 1, 1, 0])), 0x102: (1, r.choice([1, 1, 1, 0])),
        0x7B4: (4, BUF47), 0x7B8: (4, BUF47), 0x7BC: (4, n | (g(16) << 16 if r.random() < 0.2 else 0)),
        0x558: (4, banks), 0x1BC: (4, sh), 0x1C0: (4, (1 << sh) if r.random() < 0.8 else r.choice([1, 2, 4, 16])),
        0x60: (4, r.choice([0, 0, 100, 2048, 0xFFFFFF00, g(32)])),
        0x120: (4, r.choice([0, 0, 100, 2048, 0xFFFFFF00, g(32)])),
        0x50: (8, r.choice([1, 3, 1000, 123456, g(20), (1 << 64) - 5])),
        0x110: (8, r.choice([1, 3, 1000, 123456, g(20), (1 << 64) - 5])),
        0x20: (2, r.choice([1, 25, 50, 100, 1000])), 0x27E: (2, r.choice([0, 50, 600, 2000, 0x8000, g(16)])),
        0x27C: (1, r.choice([0, 1, 2, 2, 2])), 0x590: (8, r.choice([1, 1000, 10 ** 6, 10 ** 6 + 1, 5 * 10 ** 8, 2 * 10 ** 12])),
        0x1C: (2, r.choice([1, 10, 100, 1000, 0])),
        0x86A: (2, r.choice([0, 1, 4, 9, 0xFFFF])), 0x86C: (2, r.choice([0, 1, 4, 9])),
        0x86E: (2, r.choice([1, 1, 2, 5, 10])), 0x870: (2, r.choice([1, 2, 5, 10])),
        0xB28 + 0x18: (4, r.choice([0, 5, 50, 1000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF])),
        0xB44 + 0x18: (4, r.choice([0, 5, 50, 1000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF])),
    }
    sums = bytes(g(8) if r.random() < 0.3 else 0 for _ in range(0xC0))

    def setup(m):
        m.write(BUF47, data)
        m.write(SYS + 0xA68, sums)
        for o, (sz, v) in words.items():
            m.write(SYS + o, (v & ((1 << (8 * sz)) - 1)).to_bytes(sz, "little"))
    return Case([ch], setup=setup, ret=0)


TESTS[0x40032E0C] = t_meas47


# ================================================================ batch 47b: measurement display
OUT47 = SYS + 0x270000


def mag47(r):
    e = r.randrange(0, 19)
    base = 10 ** e
    v = r.choice([base, base - 1, base + 1, r.randrange(base, base * 10), r.getrandbits(r.choice([8, 20, 40, 63])),
                  base // 2, base // 2 + 1, 0, 5, 50, 51, 501])
    if e >= 4 and r.random() < 0.3:
        d = 10 ** (e - 3)
        v = r.randrange(base, base * 10) // d * d + d // 2 + r.choice([0, 1])
    if r.random() < 0.4:
        v = -v
    if r.random() < 0.02:
        v = -2 ** 63
    return v & 0xFFFFFFFFFFFFFFFF


def t_sisc47(r):
    v = mag47(r)
    kind = r.choice(list(range(0, 14)) + [r.getrandbits(32)])
    if r.random() < 0.5:
        e = r.randrange(4, 14)
        d = 10 ** (e - 3)
        v = r.randrange(10 ** e, 10 ** (e + 1)) // d * d + d // 2 + r.choice([0, 1])
        kind = r.choice([1, 3, 6, 7, 8, 9, 10, 7, 8, 9, 10])
        if r.random() < 0.3:
            v = -v
        v &= 0xFFFFFFFFFFFFFFFF
    d0, p0 = r.getrandbits(32), r.getrandbits(32)

    def setup(m):
        m.w32(OUT47 + 8, d0)
        m.w32(OUT47 + 12, p0)
    return Case([kind, r.getrandbits(32), v & 0xFFFFFFFF, v >> 32], setup=setup, ret=0,
                stack=[OUT47, OUT47 + 8, OUT47 + 12])


TESTS[0x400301E0] = t_sisc47


def text_be4(r, name, addr):
    seq = [r.getrandbits(16) for _ in range(8)]

    def fn(m, a):
        a = regs(m, 3)
        b = m.read(a[2], 8)
        m.calls.append((name, a[0], a[1], b[:b.index(0) if 0 in b else 8]))
        m.write(0x42E68BE4, seq[m.nt % 8].to_bytes(2, "little"))
        m.nt += 1
    return Stub(name, addr, fn, 0)


def t_mitem47(r):
    vals = b"".join(mag47(r).to_bytes(8, "little") for _ in range(24))
    be4 = r.getrandbits(16)

    def setup(m):
        m.nt = 0
        m.write(SYS + 0x9A8, vals)
        m.write(0x42E68BE4, be4.to_bytes(2, "little"))
    return Case([r.getrandbits(r.choice([10, 16, 32])), r.getrandbits(r.choice([10, 16, 32])),
                 r.choice([0, 0, 1, 1, 2, 0x100]), r.choice(list(range(1, 13)) + [7, 7, 8])], setup=setup, ret=0,
                stubs=(rec("half", 0x4002DC40, 1), rec("colour", 0x4002EA38, 1), rec("font", 0x4002CCCC, 1),
                       text_be4(r, "text", 0x4002CE7C), rec_stk("num", 0x4002D6F8, 1)))


TESTS[0x40030AF8] = t_mitem47


def t_mpanel47(r):
    n = r.choice([0, 1, 2, 5, 6, 7, 11, 12, 18, 23, 24])
    items = [r.choice([1, -1]) * r.randrange(1, 13) for _ in range(n)] + [0] * (24 - n)
    if r.random() < 0.1:
        items[r.randrange(24)] = r.choice([0x7FFF, -0x8000, 0])
    data = b"".join((v & 0xFFFF).to_bytes(2, "little") for v in items) + bytes([r.getrandbits(8), r.getrandbits(8)])
    t17 = r.choice([0, 0, 0, 0, 1])
    s29 = r.choice([0, 0, 0, 0, 1])

    def setup(m):
        m.write(SYS + 0x880, data)
        m.w8(0x42E68BB0 + 0x17, t17)
        m.w8(SYS + 0x29, s29)
    return Case([], setup=setup, ret=0,
                stubs=(rec("colour", 0x4002EA38, 1), rec("rect", 0x4002ECC4, 4), rec("span", 0x4002EBA8, 3),
                       rec("item", 0x40030AF8, 4)))


TESTS[0x40030F8C] = t_mpanel47


# ================================================================ batch 47c: manual cursors
def t_mcur47(r):
    g = r.getrandbits
    st = geom46(r)
    f = {0x652: r.choice([0, 0, 0, 0, 1]), 0x28: r.choice([0, 0, 1]), 0x3C: r.choice([0, 1, 1]),
         0x802: r.choice([0, 1, 1]), 0x803: r.choice([0, 1, 1]), 0x2AA: r.choice([0, 0, 1])}
    w = {o: r.choice([0, 3, 100, 400, g(10), g(32), 0xFFFFFFF0]) for o in (0x808, 0x80C, 0x810, 0x814)}
    t17 = r.choice([0, 0, 0, 0, 1])

    def setup(m):
        st(m)
        for o, v in f.items():
            m.w8(SYS + o, v)
        for o, v in w.items():
            m.w32(SYS + o, v)
        m.w8(0x42E68BB0 + 0x17, t17)
    return Case([], setup=setup, ret=0,
                stubs=(rec("colour", 0x4002EA38, 1), rec("rect", 0x4002ECC4, 4), rec("font", 0x4002CCCC, 1),
                       rec("half", 0x4002DC40, 1), str_stub("text", 0x4002CE7C, si=2), rec("span", 0x4002EBA8, 3),
                       rec("vline", 0x4002EBC8, 3)))


TESTS[0x4003117C] = t_mcur47


# ================================================================ batch 48: saved files
def t_path48(r):
    n = r.choice([-9, -1, 0, 5, 9, 10, 42, 99, 100, 512, 999, 1000, 4321, 9999, 10000, 54321, 65535, 65536,
                  r.randrange(0, 70000), r.randrange(0, 70000), r.getrandbits(32)])
    junk = bytes(r.getrandbits(8) for _ in range(32))

    def setup(m):
        m.write(SYS + 0x88C0, junk)
    return Case([n & 0xFFFFFFFF], setup=setup, ret=0)


TESTS[0x40034B78] = t_path48
TESTS[0x40035BC4] = t_path48

FB48 = 0x41FB8FE8


def t_wthumb48(r):
    x0, w = r.randrange(0, 1000), r.choice([0, 1, 4, 5, r.randrange(0, 40)])
    y0, h = r.randrange(0, 580), r.choice([0, 1, 3, 4, r.randrange(0, 30)])
    if r.random() < 0.1:
        w, h = r.randrange(0, 6), r.randrange(100, 300)
    elif r.random() < 0.3:
        x0, w, h = r.randrange(0, 100), r.randrange(200, 1000), r.randrange(0, 8)
    hdr = r.getrandbits(32)
    pix = bytes(r.getrandbits(8) for _ in range(2044 * (h + 6)))

    def setup(m):
        m.max_insns = 20_000_000
        m.write(SYS + 4, b"".join(v.to_bytes(2, "little") for v in (x0, y0, w, h)))
        m.w32(SYS + 0x109684, hdr)
        m.write(FB48 + 2044 * y0, pix)
    return Case([], setup=setup, ret=0)


TESTS[0x40035EE0] = t_wthumb48


def t_strace48(r):
    idx = r.randrange(0, 4)
    n = r.randrange(1, 9)
    w = r.randrange(0, 400)
    start = r.randrange(w // 2, w // 2 + 300)
    end = start + r.choice([-2, 0, 1, r.randrange(0, 40)])
    base38c = r.randrange(0, 3000)
    div = r.choice([1, 1, 2, 3, 16, r.randrange(1, 40)])
    a = 0x411AF3E8 + 0x927C0 * idx + 2 * (base38c + n * (start - w // 2))
    data = bytes(r.getrandbits(8) for _ in range(2 * n * (max(end - start, 0) + 2)))
    hA = r.getrandbits(16)

    def setup(m):
        m.w8(SYS + 0x3DA530, idx)
        m.w32(SYS + 0x3DA534, start)
        m.w32(SYS + 0x3DA538, end & 0xFFFFFFFF)
        m.w32(SYS + 0x5F8, n)
        m.w32(SYS + 0x38C, base38c)
        m.write(SYS + 8, w.to_bytes(2, "little"))
        m.write(SYS + 0xA, hA.to_bytes(2, "little"))
        m.w32(SYS + 0x1C0, div)
        m.write(a, data)
    return Case([], setup=setup, ret=0)


TESTS[0x40036864] = t_strace48


def recs48(r):
    cnt, rs = r.randrange(0, 7), r.randrange(1, 9)
    cells = []
    for _ in range(cnt):
        rec = [r.getrandbits(16) for _ in range(rs)]
        if r.random() < 0.3:
            rec[0] &= 0xFF00
            if rs > 1:
                rec[1] &= 0xFF00
        cells += rec
    return cnt, rs, cells


def t_rapp48(r):
    cnt, rs, cells = recs48(r)
    ln = r.randrange(0, 12)
    idv = r.getrandbits(16)
    ln |= r.getrandbits(1) << 8

    def setup(m):
        m.write(SYS + 0x758, cnt.to_bytes(2, "little"))
        m.w32(SYS + 0x3DA544, rs)
        m.w32(SYS + 0x3DA53C, ln)
        m.write(SYS + 0xF9C, idv.to_bytes(2, "little"))
        m.write(SYS + 0xD744, b"".join(c.to_bytes(2, "little") for c in cells))
    return Case([r.getrandbits(32), r.getrandbits(32), r.randrange(0, 60)], setup=setup, ret=0,
                stubs=(rec("copy", 0x40035728, 3),))


TESTS[0x40036DE4] = t_rapp48


def t_rclr48(r):
    cnt, rs, cells = recs48(r)
    ids = [((cells[i] & 0xFF) << 8) | (cells[i + 1] & 0xFF if i + 1 < len(cells) else 0) for i in range(0, len(cells), rs)]
    tail = bytes(r.getrandbits(8) for _ in range(8))
    key = r.choice(ids + [0, r.getrandbits(16)]) if ids and r.random() < 0.7 else r.getrandbits(r.choice([8, 16, 32]))

    def setup(m):
        m.write(SYS + 0x758, cnt.to_bytes(2, "little"))
        m.w32(SYS + 0x3DA544, rs)
        m.write(SYS + 0xD744, b"".join(c.to_bytes(2, "little") for c in cells) + tail)
    return Case([key, r.getrandbits(32), r.getrandbits(32), r.randrange(0, 60)], setup=setup, ret=0,
                stubs=(rec("copy", 0x40035728, 3),))


TESTS[0x40037020] = t_rclr48


# ================================================================ batch 49: .bmp file, trace zoom, list C preview
def t_bmp49(r):
    w, h = r.randrange(0, 210), r.randrange(0, 140)
    if r.random() < 0.3:
        w, h = r.randrange(0, 1100), r.randrange(0, 700)
    wrc = r.choice([0, 0, 1, 7])
    fs = fs_setup(r, 1)
    path = r.getrandbits(32)

    def setup(m):
        fs(m)
        m.wrc = wrc
        m.write(SYS + 0x109684, w.to_bytes(2, "little") + h.to_bytes(2, "little"))
    return Case([path], setup=setup, ret=R,
                stubs=fs_stubs() + (rec("memcpy", 0x40035724, 3), rec("dtd", 0x400342CC, 0), rec("fbb", 0x400342DC, 0)))


TESTS[0x4005C3F0] = t_bmp49

SRC49 = SYS + 0x200000


def t_zoom49(r):
    k = r.choice([1, 2, 10, r.randrange(1, 3000)])
    m_ = r.choice([k, k - 1, k + 1, k * 2, k * 10, k + r.randrange(1, 5000), r.randrange(1, 6000)]) & 0xFFFFFFFF
    w = r.choice([1, 2, 3, 193, r.randrange(1, 400)])
    center = r.randrange(2000, 16000)
    shift = r.choice([0, 0, 1, -1, r.randrange(-60, 60)])
    data = bytes(r.getrandbits(8) for _ in range(40000))
    if r.random() < 0.2:
        data = b"".join(v.to_bytes(2, "little") for v in [r.choice([0, 0xFFFF, 0x8000, 100])] * 20000)

    def setup(m):
        m.w32(SYS + 0x554, k)
        m.write(SRC49, data)
    return Case([SRC49, center, w, m_], setup=setup, ret=0, stack=[shift & 0xFFFFFFFF, r.getrandbits(32)],
                stubs=(rec("copy", 0x40035728, 3),))


TESTS[0x4005C678] = t_zoom49


def t_lcprev49(r):
    ln = r.choice([1, 2, 3, 192, 193, 194, 386, r.randrange(1, 2500)])
    k = r.choice([1, 10, 500, r.randrange(1, 4000)])
    cells = [r.choice([r.getrandbits(16), r.getrandbits(8), 1000 + r.randrange(50)]) for _ in range(ln)]
    if r.random() < 0.1:
        cells = [r.getrandbits(16)] * ln
    zb = bytes(r.getrandbits(8) for _ in range(0x9C40 + 0x400))
    a0 = r.getrandbits(32)

    def setup(m):
        m.max_insns = 20_000_000
        m.w32(SYS + 0x3DA53C, ln)
        m.w32(SYS + 0x554, k)
        m.write(SYS + 0x3DA548, b"".join(c.to_bytes(2, "little") for c in cells))
        m.write(0x416431E8 - 0x200, zb)
    return Case([a0], setup=setup, ret=0,
                stubs=(rec("select", 0x40036EA4, 1), rec("decimate", 0x400374EC, 1)))


TESTS[0x4003754C] = t_lcprev49


def t_num49(r):
    n = r.choice([-9, -1, 0, 5, 9, 10, 42, 99, 100, 512, 999, 1000, 4321, 9999, 10000, 54321, 65535, 65536,
                  r.randrange(0, 70000), r.randrange(0, 70000), r.getrandbits(32)])
    junk = bytes(r.getrandbits(8) for _ in range(16))

    def setup(m):
        m.write(SYS + 0x88C0, junk)
    return Case([n & 0xFFFFFFFF], setup=setup, ret=0)


TESTS[0x40037160] = t_num49
