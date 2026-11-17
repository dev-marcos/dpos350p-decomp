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
