"""Behavioural tests for the FatFs R0.13b functions (stock vs nonmatching C).

Every test is a function case(rng) -> Case. The runner builds two fresh machines, applies
the same setup to both, calls the stock address and the C build, and compares outcomes.
"""
from tools.emu.emu import Stub

BUF = 0x41000000      # scratch buffers inside the stock ZI region (unused during a unit test)
BUF2 = 0x41010000
FS = 0x41020000       # a FATFS object
N = 300               # cases per function


class Case:
    def __init__(self, args, setup=None, ret=32, stubs=(), stack=()):
        self.args, self.setup, self.ret, self.stubs, self.stack = args, setup, ret, stubs, stack


def rand_bytes(rng, n):
    return bytes(rng.getrandbits(8) for _ in range(n))


# ---------------------------------------------------------------- disk stubs
def disk_stubs(read_ok=True, write_ok=True, content=None):
    """disk_read fills the buffer from 'content(sector)' and returns 0/1 (RES_OK/RES_ERROR)."""
    def rd(m, a):
        pdrv, buff, sector, count = a
        if read_ok is True or (callable(read_ok) and read_ok(sector)):
            m.write(buff, content(sector) if content else bytes(512))
            return 0
        return 1

    def wr(m, a):
        return 0 if (write_ok is True or (callable(write_ok) and write_ok(a[2]))) else 1
    return (Stub("disk_read", 0x40049C78, rd), Stub("disk_write", 0x40049CE8, wr))


def fatfs_obj(rng, wflag=None, winsect=None, n_fats=None):
    o = bytearray(rand_bytes(rng, 0x38 + 512))
    o[3] = rng.choice([0, 1]) if wflag is None else wflag
    o[2] = rng.choice([1, 2]) if n_fats is None else n_fats
    o[1] = rng.randrange(4)
    fsize = rng.choice([0, 1, 8, 0x100, 0x2000])
    fatbase = rng.choice([0, 32, 0x800, 0x10000])
    ws = rng.choice([fatbase, fatbase + 1, fatbase + fsize, fatbase + fsize + 3, rng.getrandbits(32)]) \
        if winsect is None else winsect
    o[0x20:0x24] = (fsize & 0xFFFFFFFF).to_bytes(4, "little")
    o[0x28:0x2C] = (fatbase & 0xFFFFFFFF).to_bytes(4, "little")
    o[0x34:0x38] = (ws & 0xFFFFFFFF).to_bytes(4, "little")
    return bytes(o)


# ---------------------------------------------------------------- cases
def t_ld(n):
    def case(rng):
        data = rand_bytes(rng, 16)
        return Case([BUF + rng.randrange(8)], lambda m: m.write(BUF, data), ret=n)
    return case


def t_st(n):
    def case(rng):
        val = rng.getrandbits(64)
        off = rng.randrange(8)
        args = [BUF + off, val & 0xFFFFFFFF] if n <= 32 else [BUF + off, 0, val & 0xFFFFFFFF, val >> 32]
        return Case(args, lambda m: m.write(BUF, bytes(32)), ret=0)
    return case


def t_mem_cpy(rng):
    src = rand_bytes(rng, 300)
    cnt = rng.choice([0, 1, 2, 7, 64, rng.randrange(256)])
    return Case([BUF2, BUF, cnt], lambda m: (m.write(BUF, src), m.write(BUF2, bytes(300))), ret=0)


def t_mem_set(rng):
    cnt = rng.choice([1, 2, 7, 64, rng.randrange(1, 256)])
    return Case([BUF, rng.getrandbits(32), cnt], lambda m: m.write(BUF, bytes(i & 0xFF for i in range(300))), ret=0)


def t_mem_cmp(rng):
    a = rand_bytes(rng, 64)
    b = bytearray(a)
    if rng.random() < 0.7:
        b[rng.randrange(64)] = rng.getrandbits(8)
    cnt = rng.choice([1, 2, 11, 64, rng.randrange(1, 64)])
    return Case([BUF, BUF2, cnt], lambda m: (m.write(BUF, a), m.write(BUF2, bytes(b))))


def t_chk_chr(rng):
    s = bytes(rng.choice(b"abcAB:.\\/*?<>|\x80\xff") for _ in range(rng.randrange(0, 12))) + b"\0"
    chr_ = rng.choice([0, ord(":"), ord("*"), 0x80, -128, ord("a"), rng.getrandbits(8), rng.getrandbits(32)])
    return Case([BUF, chr_], lambda m: m.write(BUF, s))


def t_dbc(rng):
    return Case([rng.choice([0, 1, 0x80, 0xFF, rng.getrandbits(32)])])


def conv_stub(name, addr, rng):
    """Replace the code-page converter with arbitrary results, so that branches unreachable with
    CP437 (DBCS results, conversion failures) are exercised identically in both versions."""
    vals = [0, 0x41, 0xFF, 0x100, 0x8140, 0xFFFF, rng.getrandbits(16)]
    pick = rng.choice(vals)
    return Stub(name, addr, lambda m, a: pick, nargs=2)


def t_tchar2uni(rng):
    s = bytes(rng.choice([0, 0x41, 0x7F, 0x80, 0x9A, 0xE1, 0xFF, rng.getrandbits(8)]) for _ in range(4))
    stubs = (conv_stub("ff_oem2uni", 0x40069DF4, rng),) if rng.random() < 0.5 else ()
    return Case([BUF2], lambda m: (m.write(BUF, s), m.w32(BUF2, BUF)), stubs=stubs)


def t_put_utf(rng):
    chr_ = rng.choice([0, 0x41, 0x7F, 0xC7, 0xFC, 0x3B1, 0x2591, 0xFFFF, 0x1F600, rng.getrandbits(16)])
    szb = rng.choice([0, 1, 2, 3])
    stubs = (conv_stub("ff_uni2oem", 0x40069D90, rng),) if rng.random() < 0.5 else ()
    return Case([chr_, BUF, szb], lambda m: m.write(BUF, bytes(8)), ret=8, stubs=stubs)


def t_sync_window(rng):
    obj = fatfs_obj(rng)
    ok = rng.random() < 0.8
    return Case([FS], lambda m: m.write(FS, obj), ret=8, stubs=disk_stubs(write_ok=ok))


def t_move_window(rng):
    obj = fatfs_obj(rng)
    ws = int.from_bytes(obj[0x34:0x38], "little")
    sector = rng.choice([ws, ws + 1, rng.getrandbits(32), 0xFFFFFFFF])
    rok, wok = rng.random() < 0.8, rng.random() < 0.8
    content = lambda s: bytes(((s * 7 + i) & 0xFF) for i in range(512))
    return Case([FS, sector], lambda m: m.write(FS, obj), ret=8,
                stubs=disk_stubs(read_ok=rok, write_ok=wok, content=content))


def boot_sector(rng):
    kind = rng.choice(["exfat", "fat", "fat32", "fat3x", "fax", "nofat", "nosig", "jmpbad", "random"])
    b = bytearray(rand_bytes(rng, 512))
    b[510], b[511] = 0x55, 0xAA
    if kind == "exfat":
        b[0:11] = b"\xEB\x76\x90EXFAT   "
    elif kind == "fat":
        b[0] = rng.choice([0xE9, 0xEB, 0xE8]); b[54:57] = b"FAT"
    elif kind == "fat32":
        b[0] = rng.choice([0xE9, 0xEB, 0xE8]); b[82:87] = b"FAT32"
    elif kind == "fat3x":   # "FAT3" + something other than '2'
        b[0] = rng.choice([0xE9, 0xEB, 0xE8]); b[82:86] = b"FAT3"; b[86] = rng.choice(b"013x")
        b[54:57] = b"FAx"
    elif kind == "fax":     # near-miss of "FAT"
        b[0] = rng.choice([0xE9, 0xEB, 0xE8]); b[54:56] = b"FA"; b[56] = rng.choice(b"Xt\x00")
    elif kind == "nofat":
        b[0] = rng.choice([0xE9, 0xEB, 0xE8])
    elif kind == "nosig":
        b[510] = rng.getrandbits(8)
    elif kind == "jmpbad":
        b[0] = 0x00; b[54:57] = b"FAT"
    return bytes(b)


def t_check_fs(rng):
    obj = fatfs_obj(rng, wflag=rng.choice([0, 1]))
    bs = boot_sector(rng)
    rok = rng.random() < 0.85
    return Case([FS, rng.getrandbits(32)], lambda m: m.write(FS, obj), ret=8,
                stubs=disk_stubs(read_ok=rok, content=lambda s: bs))


TESTS = {
    0x40056A70: t_ld(16), 0x40056A80: t_ld(32), 0x40056AA0: t_ld(64),
    0x40056B04: t_st(16), 0x40056B14: t_st(32), 0x40056B34: t_st(64),
    0x40056B90: t_mem_cpy, 0x40056BAC: t_mem_set, 0x40056BBC: t_mem_cmp,
    0x40056BE0: t_chk_chr, 0x40056C00: t_dbc, 0x40056C08: t_dbc,
    0x40056C10: t_tchar2uni, 0x40056C84: t_put_utf,
    0x40056CF0: t_sync_window, 0x40056D80: t_move_window, 0x40059454: t_check_fs,
}
