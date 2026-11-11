"""Scenario tests for the FatFs R0.13b filesystem layer (stock vs nonmatching C).

Each case picks a fixture volume (FAT12/16/32/exFAT, formatted and populated by the stock
code), mounts it with the stock f_mount, prepares objects with stock calls, and then the
runner calls the function under test (stock address, then the C build) from that state.
Compared: return value, final memory outside the callee frame, and the disk I/O sequence
(sector, count, data hash) -- see tools/emu/fatdisk.py.
"""
import random

from tools.emu import fatdisk as F
from tools.emu.tests.fatfs import Case

N = 40
VOLS = list(F.VOLUMES)
EXISTING = ["README.TXT", "a.bin", "Long File Name With Spaces.dat", "dir1/sub/deep.txt", "dir1/x.c",
            "mixedCase.Txt", "frag1.bin", "frag2.bin", "empty.dat", "dir1/file07_with_a_longer_name.log",
            "dir1/file39_with_a_longer_name.log"]
DIRS = ["", "dir1", "dir1/sub", "UPPER", "/", "dir1/"]
NEW = ["new.txt", "NEW2.BIN", "A very long new name for testing LFN.txt", "dir1/new_in_dir.dat",
       "dir1/sub/n", "x" * 12 + ".abc", "spaces in name .txt", "MiXeD.CaSe", "~1.txt", "a+b,c;d=e[f].g"]
BAD = ["nosuchdir/x.txt", "dir1/nosuch/y", "bad|name.txt", "", "1:/x", "a\\b\\c", "..", "dir1/../x",
       "con", "toolong" * 40]

INTERNAL = {  # stock addresses of internal functions used for scenario preparation
    "dir_sdi": 0x400579D4, "dir_next": 0x40057AEC, "dir_read": 0x4005860C, "follow_path": 0x400592B8,
    "create_name": 0x40058F8C, "find_volume": 0x40059520, "load_xdir": 0x40058284,
}
PPATH = F.SCR + 0x800        # TCHAR* variable (for create_name/follow_path/find_volume)
PFS = F.SCR + 0x810          # FATFS* out variable
OUT = F.SCR + 0x820          # UINT / DWORD out variable


def env_for(code, vol, disk=None, **kw):
    e = F.Env(disk or F.volume(vol), code=code, **kw)
    if e.mount():
        raise RuntimeError("stock mount failed")
    return e


def scenario(rng, prep, vol=None, ret=32, **kw):
    """Case whose machine is built by: mount 'vol', then prep(env) (stock calls only)."""
    vol = vol or rng.choice(VOLS)
    seed = rng.getrandbits(32)

    def build(code):
        e = env_for(code, vol, **kw)
        prep(e, random.Random(seed))
        return e.m
    c = Case([], ret=ret)
    c.build = build
    return c


def with_args(c, args, stack=()):
    c.args, c.stack = args, stack
    return c


def ffs(e):                 # FATFS fields used to pick valid parameters
    m = e.m
    return {"fs_type": m.read(F.FS, 1)[0], "n_fatent": m.r32(F.FS + 0x1C), "csize": int.from_bytes(m.read(F.FS + 0x0A, 2), "little")}


def clusters(rng, n_fatent):
    return rng.choice([0, 1, 2, 3, n_fatent - 1, n_fatent, n_fatent + 5, rng.randrange(2, max(3, n_fatent)),
                       0xFFFFFFFF, 0x0FFFFFF7])


def opened(e, path, mode=F.FA_READ, fil=F.FIL1):
    return e.open(fil, path, mode)


def follow(e, path, dp=F.DIR1):
    """Stock follow_path on a fresh DIR (dp->obj.fs = fs), like f_open does."""
    e.m.w32(dp, F.FS)
    F.cstr(e.m, F.STR, path)
    e.m.w32(PPATH, F.STR)
    r = e.m.call(INTERNAL["follow_path"], [dp, F.STR])
    return r["r0"]


def void(case_fn):
    """Test decorator for functions returning void: r0 is not compared."""
    def wrapped(rng):
        c = case_fn(rng)
        c.ret = 0
        return c
    return wrapped


# ---------------------------------------------------------------- cluster / FAT layer
def t_sync_fs(rng):
    def prep(e, r):
        e.write_file(r.choice(NEW), b"q" * r.randrange(1, 3000), mode=F.FA_CREATE_ALWAYS | F.FA_WRITE)
        e.m.write(F.FS + 3, bytes([r.choice([0, 1])]))      # wflag
        e.m.write(F.FS + 4, bytes([r.choice([0, 1, 0x80, 0x81])]))  # fsi_flag
    return with_args(scenario(rng, prep), [F.FS])


def t_clst2sect(rng):
    c = scenario(rng, lambda e, r: None)
    c.args = [F.FS, rng.choice([0, 1, 2, 3, 100, 65000, 0x0FFFFFFF, rng.getrandbits(17)])]
    return c


def t_get_fat(rng):
    path = rng.choice(EXISTING)
    clst = rng.getrandbits(32)

    def prep(e, r):
        opened(e, path)
    c = scenario(rng, prep)
    c.args = [F.FIL1 + F.FIL_OBJ, rng.choice([0, 1, 2, 3, 4, 5, 9, 100, 4000, 66000, 0xFFFFFFFF, clst & 0xFFFF])]
    return c


def t_put_fat(rng):
    c = scenario(rng, lambda e, r: None)
    c.args = [F.FS, rng.choice([0, 1, 2, 3, 50, 4000, 66000, rng.getrandbits(16)]),
              rng.choice([0, 1, 0xFFFFFFFF, 0x0FFFFFFF, 7, rng.getrandbits(32)])]
    return c


def t_find_bitmap(rng):
    c = scenario(rng, lambda e, r: None, vol="exfat")
    c.args = [F.FS, rng.choice([0, 1, 2, 3, 100, 20000, 0xFFFFFFFF]), rng.choice([1, 2, 5, 64, 1000])]
    return c


def t_change_bitmap(rng):
    c = scenario(rng, lambda e, r: None, vol="exfat")
    c.args = [F.FS, rng.choice([2, 3, 100, 5000, 16000]), rng.choice([1, 2, 8, 33, 700]), rng.choice([0, 1])]
    return c


def frag_obj(rng, prep_mode=F.FA_READ | F.FA_WRITE):
    path = rng.choice(["frag1.bin", "frag2.bin", "a.bin", "README.TXT", "empty.dat"])
    return path


def t_fill_first_frag(rng):
    path = frag_obj(rng)

    def prep(e, r):
        opened(e, path, F.FA_READ | F.FA_WRITE)
        e.m.write(F.FIL1 + 7, bytes([r.choice([0, 2, 3])]))           # obj.stat
        e.m.w32(F.FIL1 + 0x18, r.choice([0, 1, 3, 10]))               # n_cont
    return with_args(scenario(rng, prep, vol="exfat"), [F.FIL1 + F.FIL_OBJ])


def t_fill_last_frag(rng):
    path = frag_obj(rng)

    def prep(e, r):
        opened(e, path, F.FA_READ | F.FA_WRITE)
        e.m.w32(F.FIL1 + 0x1C, r.choice([0, 1, 2, 5]))                # n_frag
    c = scenario(rng, prep, vol="exfat")
    c.args = [F.FIL1 + F.FIL_OBJ, rng.choice([10, 100, 2000]), rng.choice([0xFFFFFFFF, 0, 7])]
    return c


def t_remove_chain(rng):
    path = frag_obj(rng)

    def prep(e, r):
        opened(e, path, F.FA_READ | F.FA_WRITE)
    c = scenario(rng, prep)
    sclust_hint = rng.choice(["sclust", "bad0", "bad1", "big"])

    def args_from(e):
        return e.m.r32(F.FIL1 + 8)
    c.args = None
    orig_build = c.build

    def build(code):
        m = orig_build(code)
        scl = m.r32(F.FIL1 + 8)
        c.args = [F.FIL1, {"sclust": scl, "bad0": 0, "bad1": 1, "big": 0x7FFFFFF0}[sclust_hint],
                  rng_choice_p]
        return m
    rng_choice_p = rng.choice([0, 0, 2])
    c.build = build
    return c


def t_create_chain(rng):
    path = frag_obj(rng)
    mode = rng.choice(["new", "extend", "bad"])

    def prep(e, r):
        opened(e, path, F.FA_READ | F.FA_WRITE)
    c = scenario(rng, prep)
    orig_build = c.build

    def build(code):
        m = orig_build(code)
        scl = m.r32(F.FIL1 + 8)
        c.args = [F.FIL1, {"new": 0, "extend": scl, "bad": 0xFFFFFFFF}[mode]]
        return m
    c.build = build
    return c


# ---------------------------------------------------------------- directory layer
def opendir(e, path, dp=F.DIR1):
    """Stock f_opendir; falls back to the root so the DIR is always in a valid state."""
    F.cstr(e.m, F.STR, path)
    r = e.stock("f_opendir", dp, F.STR)
    if r:
        F.cstr(e.m, F.STR, "")
        r = e.stock("f_opendir", dp, F.STR)
    return r


def t_dir_clear(rng):
    c = scenario(rng, lambda e, r: None)
    c.args = [F.FS, rng.choice([2, 3, 5, 200, 3000])]
    return c


def t_dir_sdi(rng):
    d = rng.choice(DIRS)
    c = scenario(rng, lambda e, r: opendir(e, d))
    c.args = [F.DIR1, rng.choice([0, 32, 64, 512, 4096, 32 * 41, 0x1FFFE0, 0x200000, 33, rng.randrange(0, 8192, 32)])]
    return c


def t_dir_next(rng):
    d = rng.choice(DIRS)
    steps = rng.randrange(0, 60)

    def prep(e, r):
        opendir(e, d)
        for _ in range(steps):
            if e.m.call(INTERNAL["dir_next"], [F.DIR1, 0])["r0"]:
                break
    c = scenario(rng, prep)
    c.args = [F.DIR1, rng.choice([0, 1])]
    return c


def t_dir_alloc(rng):
    d = rng.choice(DIRS)
    c = scenario(rng, lambda e, r: opendir(e, d))
    c.args = [F.DIR1, rng.choice([1, 2, 3, 19, 40])]
    return c


def t_ld_clust(rng):
    ent = bytes(rng.getrandbits(8) for _ in range(32))
    c = scenario(rng, lambda e, r: e.m.write(F.SCR, ent))
    c.args = [F.FS, F.SCR]
    return c


@void
def t_st_clust(rng):
    ent = bytes(rng.getrandbits(8) for _ in range(32))
    c = scenario(rng, lambda e, r: e.m.write(F.SCR, ent))
    c.args = [F.FS, F.SCR, rng.getrandbits(32)]
    return c


def lfn_entry(rng):
    e = bytearray(rng.getrandbits(8) for _ in range(32))
    e[11] = 0x0F; e[26] = 0; e[27] = 0
    e[0] = rng.choice([1, 2, 0x41, 0x42, 0x54, rng.getrandbits(8)])
    return bytes(e)


def lfnbuf(rng):
    s = rng.choice(["readme.txt", "Long File Name With Spaces.dat", "a", "ÄÖÜ", "x" * 255, ""])
    w = [ord(ch) & 0xFFFF for ch in s][:255] + [0]
    return b"".join(x.to_bytes(2, "little") for x in w) + bytes(64)


def t_cmp_lfn(rng):
    ent, buf = lfn_entry(rng), lfnbuf(rng)
    c = scenario(rng, lambda e, r: (e.m.write(F.SCR, ent), e.m.write(F.SCR + 0x100, buf)), vol="fat16")
    c.args = [F.SCR + 0x100, F.SCR]
    return c


def t_pick_lfn(rng):
    ent = lfn_entry(rng)
    c = scenario(rng, lambda e, r: (e.m.write(F.SCR, ent), e.m.write(F.SCR + 0x100, bytes(600))), vol="fat16")
    c.args = [F.SCR + 0x100, F.SCR]
    return c


@void
def t_put_lfn(rng):
    buf = lfnbuf(rng)
    c = scenario(rng, lambda e, r: (e.m.write(F.SCR, bytes(32)), e.m.write(F.SCR + 0x100, buf)), vol="fat16")
    c.args = [F.SCR + 0x100, F.SCR, rng.choice([1, 2, 3, 20]), rng.getrandbits(8)]
    return c


@void
def t_gen_numname(rng):
    sfn = rng.choice([b"README  TXT", b"LONGFI~1DAT", b"A          ", b"12345678ABC", b"\xe5BCDEFGHIJK"])
    buf = lfnbuf(rng)
    c = scenario(rng, lambda e, r: (e.m.write(F.SCR + 0x40, sfn), e.m.write(F.SCR + 0x100, buf)), vol="fat16")
    c.args = [F.SCR, F.SCR + 0x40, F.SCR + 0x100, rng.choice([1, 2, 5, 6, 9, 100, 99999, 0x12345])]
    return c


def t_pure_buf(nbytes, extra=()):
    def case(rng):
        data = bytes(rng.getrandbits(8) for _ in range(nbytes))
        c = scenario(rng, lambda e, r: e.m.write(F.SCR, data), vol="fat16")
        c.args = [F.SCR] + [x(rng) if callable(x) else x for x in extra]
        return c
    return case


def t_xdir_sum(rng):
    data = bytearray(rng.getrandbits(8) for _ in range(32 * 19))
    data[1] = rng.choice([1, 2, 3, 17])                        # secondary count
    c = scenario(rng, lambda e, r: e.m.write(F.SCR, bytes(data)), vol="exfat", ret=16)
    c.args = [F.SCR]
    return c


def t_xname_sum(rng):
    buf = lfnbuf(rng)
    c = scenario(rng, lambda e, r: e.m.write(F.SCR, buf), vol="exfat", ret=16)
    c.args = [F.SCR]
    return c


def t_xsum32(rng):
    c = scenario(rng, lambda e, r: None, vol="fat16")
    c.args = [rng.getrandbits(8), rng.getrandbits(32)]
    return c


def t_get_xfileinfo(rng):
    data = bytearray(rng.getrandbits(8) for _ in range(32 * 19))
    data[1] = rng.choice([2, 3, 4]); data[32 + 3] = rng.choice([0, 1, 5, 30])
    c = scenario(rng, lambda e, r: (e.m.write(F.SCR, bytes(data)), e.m.write(F.FINFO, bytes(0x120))), vol="exfat")
    c.args = [F.SCR, F.FINFO]
    return c


def exfat_at_entry(e, r, path):
    """dp positioned on the file entry of 'path' (exFAT), as f_open/f_stat leave it."""
    res = follow(e, path)
    return res


def t_load_xdir(rng):
    path = rng.choice(EXISTING)

    def prep(e, r):
        follow(e, path)
        blk = e.m.r32(F.DIR1 + 0x4C)                       # blk_ofs
        e.m.call(INTERNAL["dir_sdi"], [F.DIR1, blk])
    return with_args(scenario(rng, prep, vol="exfat"), [F.DIR1])


@void
def t_init_alloc_info(rng):
    path = rng.choice(EXISTING)
    return with_args(scenario(rng, lambda e, r: follow(e, path), vol="exfat"), [F.FS, F.DIR1])


def t_load_obj_xdir(rng):
    path = rng.choice(EXISTING)

    def prep(e, r):
        opened(e, path)
    return with_args(scenario(rng, prep, vol="exfat"), [F.DIR1, F.FIL1])


def t_store_xdir(rng):
    path = rng.choice(EXISTING)

    def prep(e, r):
        follow(e, path)
        blk = e.m.r32(F.DIR1 + 0x4C)
        e.m.call(INTERNAL["dir_sdi"], [F.DIR1, blk])
        e.m.call(INTERNAL["load_xdir"], [F.DIR1])
    return with_args(scenario(rng, prep, vol="exfat"), [F.DIR1])


def t_create_xdir(rng):
    buf = lfnbuf(rng)
    c = scenario(rng, lambda e, r: (e.m.write(F.SCR + 0x400, buf), e.m.write(F.SCR, bytes(0x300))), vol="exfat")
    c.args = [F.SCR, F.SCR + 0x400]
    return c


def t_dir_read(rng):
    d = rng.choice(DIRS)
    steps = rng.randrange(0, 45)

    def prep(e, r):
        opendir(e, d)
        for _ in range(steps):
            if e.m.call(INTERNAL["dir_read"], [F.DIR1, 0])["r0"]:
                break
            e.m.call(INTERNAL["dir_next"], [F.DIR1, 0])
    c = scenario(rng, prep)
    c.args = [F.DIR1, rng.choice([0, 0, 1])]
    return c


def t_dir_find(rng):
    path = rng.choice(EXISTING + NEW)
    parent, _, leaf = path.rpartition("/")

    def prep(e, r):
        opendir(e, parent)
        F.cstr(e.m, F.STR, leaf)
        e.m.w32(PPATH, F.STR)
        e.m.call(INTERNAL["create_name"], [F.DIR1, PPATH])
    return with_args(scenario(rng, prep), [F.DIR1])


def t_dir_register(rng):
    path = rng.choice(NEW)
    return with_args(scenario(rng, lambda e, r: follow(e, path)), [F.DIR1])


def t_dir_remove(rng):
    path = rng.choice(EXISTING)
    return with_args(scenario(rng, lambda e, r: follow(e, path)), [F.DIR1])


@void
def t_get_fileinfo(rng):
    d = rng.choice(DIRS)
    steps = rng.randrange(0, 45)

    def prep(e, r):
        opendir(e, d)
        for _ in range(steps):
            e.m.call(INTERNAL["dir_read"], [F.DIR1, 0])
            e.m.call(INTERNAL["dir_next"], [F.DIR1, 0])
        e.m.call(INTERNAL["dir_read"], [F.DIR1, 0])
        e.m.write(F.FINFO, bytes(0x120))
    return with_args(scenario(rng, prep), [F.DIR1, F.FINFO])


def t_create_name(rng):
    name = rng.choice([p.rpartition("/")[2] for p in EXISTING + NEW] + BAD + ["a/b/c", "dir1/", "x.", ".x", " lead",
                                                                             "trail. ", "ÄÖÜ.txt", "\x7f", "a" * 300])

    def prep(e, r):
        e.m.w32(F.DIR1, F.FS)
        F.cstr(e.m, F.STR, name)
        e.m.w32(PPATH, F.STR)
    return with_args(scenario(rng, prep), [F.DIR1, PPATH])


def t_follow_path(rng):
    path = rng.choice(EXISTING + NEW + BAD + DIRS)

    def prep(e, r):
        e.m.w32(F.DIR1, F.FS)
        F.cstr(e.m, F.STR, path)
    return with_args(scenario(rng, prep), [F.DIR1, F.STR])


def t_get_ldnumber(rng):
    s = rng.choice(["0:", "0:/x", "1:", "9:abc", "x", "", ":", "00:", "0", "a:b", "/0:"])
    c = scenario(rng, lambda e, r: (F.cstr(e.m, F.STR, s), e.m.w32(PPATH, F.STR)))
    c.args = [PPATH]
    return c


# ---------------------------------------------------------------- volume / API layer
def t_find_volume(rng):
    s = rng.choice(["0:", "", "0:/dir1", "1:", "x:y"])
    mode = rng.choice([0, 1, 2, 0x13])
    unmounted = rng.random() < 0.4

    def prep(e, r):
        if unmounted:
            e.m.write(F.FS, b"\0")                        # fs_type = 0 -> forces a mount
        F.cstr(e.m, F.STR, s)
        e.m.w32(PPATH, F.STR)
    return with_args(scenario(rng, prep), [PPATH, PFS, mode])


def t_validate(rng):
    kind = rng.choice(["open", "closed", "badid", "null"])

    def prep(e, r):
        opened(e, "README.TXT")
        if kind == "closed":
            e.m.w32(F.FIL1, 0)
        elif kind == "badid":
            e.m.uc.mem_write(F.FIL1 + 4, b"\x99\x99")
    c = scenario(rng, prep)
    c.args = [0 if kind == "null" else F.FIL1, PFS]
    return c


def t_f_mount(rng):
    s = rng.choice(["0:", "", "1:", "0:/"])
    opt = rng.choice([0, 1])
    fs = rng.choice([F.FS, F.FS + 0x1000, 0])

    def prep(e, r):
        F.cstr(e.m, F.STR, s)
    return with_args(scenario(rng, prep), [fs, F.STR, opt])


def t_f_open(rng):
    path = rng.choice(EXISTING + NEW + BAD + DIRS)
    mode = rng.choice([0, 1, 2, 3, 4 | 2, 8 | 2, 0x10 | 1, 0x30 | 2, 0x13, 0xFF])

    def prep(e, r):
        F.cstr(e.m, F.STR, path)
    c = scenario(rng, prep)
    c.args = [rng.choice([F.FIL2, F.FIL2, 0]), F.STR, mode]
    return c


def t_f_read(rng):
    path = rng.choice(EXISTING)
    seek = rng.choice([0, 1, 511, 512, 1000, 5000])
    n = rng.choice([0, 1, 100, 511, 512, 513, 2048, 9000, 30000])
    mode = rng.choice([F.FA_READ, F.FA_WRITE, F.FA_READ | F.FA_WRITE])

    def prep(e, r):
        opened(e, path, mode)
        e.stock("f_lseek", F.FIL1, 0, seek, 0)
        e.m.write(F.DATA, bytes(0x8000))
    return with_args(scenario(rng, prep), [F.FIL1, F.DATA, n, OUT])


def t_f_write(rng):
    path = rng.choice(EXISTING + NEW)
    seek = rng.choice([0, 1, 511, 512, 1000, 5000, 0xFFFFFFFF])
    n = rng.choice([0, 1, 100, 512, 513, 2048, 9000, 20000])
    mode = rng.choice([F.FA_WRITE | F.FA_OPEN_ALWAYS, F.FA_READ, F.FA_WRITE | F.FA_CREATE_ALWAYS, F.FA_WRITE | F.FA_OPEN_APPEND])
    data = bytes(rng.getrandbits(8) for _ in range(256)) * 80

    def prep(e, r):
        opened(e, path, mode)
        e.stock("f_lseek", F.FIL1, 0, seek, 0)
        e.m.write(F.DATA, data[:0x8000])
    return with_args(scenario(rng, prep), [F.FIL1, F.DATA, n, OUT])


def t_f_sync(rng):
    path = rng.choice(EXISTING + NEW)
    n = rng.choice([0, 10, 700, 5000])

    def prep(e, r):
        opened(e, path, F.FA_WRITE | F.FA_OPEN_ALWAYS)
        e.m.write(F.DATA, b"s" * 5000)
        e.stock("f_write", F.FIL1, F.DATA, n, OUT)
    return with_args(scenario(rng, prep), [F.FIL1])


def t_f_close(rng):
    path = rng.choice(EXISTING + NEW)
    n = rng.choice([0, 10, 700])
    mode = rng.choice([F.FA_READ, F.FA_WRITE | F.FA_OPEN_ALWAYS])

    def prep(e, r):
        opened(e, path, mode)
        e.m.write(F.DATA, b"c" * 700)
        e.stock("f_write", F.FIL1, F.DATA, n, OUT)
    return with_args(scenario(rng, prep), [F.FIL1])


def t_f_lseek(rng):
    path = rng.choice(EXISTING)
    rel = rng.choice(["abs", "size-1", "size", "size+1", "size+600"])
    ofs = rng.choice([0, 1, 511, 512, 513, 2000, 21504, 30000, 0x100000, 0xFFFFFFFF])
    mode = rng.choice([F.FA_READ, F.FA_READ | F.FA_WRITE])

    def prep(e, r):
        opened(e, path, mode)
    c = scenario(rng, prep)
    base_build = c.build

    def build(code):
        m = base_build(code)
        size = m.r32(F.FIL1 + 0x10)                               # obj.objsize (low word)
        o = {"abs": ofs, "size-1": max(size - 1, 0), "size": size, "size+1": size + 1, "size+600": size + 600}[rel]
        c.args = [F.FIL1, 0, o & 0xFFFFFFFF, o >> 32]             # FSIZE_t (QWORD) in r2:r3
        return m
    c.build = build
    c.args = [F.FIL1, 0, 0, 0]
    return c


def t_f_opendir(rng):
    d = rng.choice(DIRS + EXISTING[:2] + BAD)
    return with_args(scenario(rng, lambda e, r: F.cstr(e.m, F.STR, d)), [rng.choice([F.DIR1, F.DIR1, 0]), F.STR])


def t_f_closedir(rng):
    d = rng.choice(DIRS)
    return with_args(scenario(rng, lambda e, r: opendir(e, d)), [F.DIR1])


def t_f_readdir(rng):
    d = rng.choice(DIRS)
    steps = rng.randrange(0, 45)

    def prep(e, r):
        opendir(e, d)
        for _ in range(steps):
            e.stock("f_readdir", F.DIR1, F.FINFO)
        e.m.write(F.FINFO, bytes(0x120))
    c = scenario(rng, prep)
    c.args = [F.DIR1, rng.choice([F.FINFO, F.FINFO, 0])]
    return c


def t_f_getfree(rng):
    s = rng.choice(["0:", "", "1:"])
    fresh = rng.random() < 0.5

    def prep(e, r):
        if fresh:
            e.m.w32(F.FS + 0x18, 0xFFFFFFFF)           # free_clst unknown -> full scan
        F.cstr(e.m, F.STR, s)
    return with_args(scenario(rng, prep), [F.STR, OUT, PFS])


def t_f_unlink(rng):
    path = rng.choice(EXISTING + DIRS + BAD + NEW + ["dir1/sub", "UPPER"])
    return with_args(scenario(rng, lambda e, r: F.cstr(e.m, F.STR, path)), [F.STR])


def t_f_mkdir(rng):
    path = rng.choice(NEW + EXISTING + DIRS + BAD + ["dir1/sub/newdir", "NEWDIR"])
    return with_args(scenario(rng, lambda e, r: F.cstr(e.m, F.STR, path)), [F.STR])


def t_f_chmod(rng):
    path = rng.choice(EXISTING + DIRS + BAD)
    return with_args(scenario(rng, lambda e, r: F.cstr(e.m, F.STR, path)),
                     [F.STR, rng.getrandbits(8), rng.choice([0x01, 0x21, 0x27, 0xFF, 0])])


def t_f_mkfs(rng):
    nsect, opt, au = rng.choice([(4096, 1 | 8, 0), (4096, 1, 0), (8192, 7, 0), (65536, 1 | 8, 2048),
                                 (131072, 2 | 8, 512), (100, 7, 0), (4096, 4 | 8, 0), (4096, 1 | 8, 512)])

    def build(code):
        e = F.Env(F.RamDisk(nsect), code=code, max_insns=400_000_000)
        F.cstr(e.m, F.STR, "0:")
        return e.m
    c = Case([F.STR, opt, au, F.WORK], ret=32, stack=(0x8000,))
    c.build = build
    return c


TESTS = {
    0x40056DD8: t_sync_fs, 0x40056EA8: t_clst2sect, 0x40056ED4: t_get_fat, 0x40057108: t_put_fat,
    0x4005728C: t_find_bitmap, 0x40057374: t_change_bitmap, 0x40057428: t_fill_first_frag,
    0x40057480: t_fill_last_frag, 0x400574D4: t_remove_chain, 0x400576B0: t_create_chain,
    0x4005793C: t_dir_clear, 0x400579D4: t_dir_sdi, 0x40057AEC: t_dir_next, 0x40057C38: t_dir_alloc,
    0x40057CDC: t_ld_clust, 0x40057D14: t_st_clust, 0x40057D50: t_cmp_lfn, 0x40057E38: t_pick_lfn,
    0x40057EF0: t_put_lfn, 0x40057F9C: t_gen_numname, 0x40058094: t_pure_buf(32),
    0x400580C4: t_xdir_sum, 0x40058118: t_xname_sum, 0x40058170: t_xsum32, 0x40058180: t_get_xfileinfo,
    0x40058284: t_load_xdir, 0x400583E4: t_init_alloc_info, 0x40058434: t_load_obj_xdir,
    0x4005848C: t_store_xdir, 0x4005853C: t_create_xdir, 0x4005860C: t_dir_read, 0x400587C8: t_dir_find,
    0x400589F8: t_dir_register, 0x40058CE4: t_dir_remove, 0x40058D7C: t_get_fileinfo,
    0x40058F8C: t_create_name, 0x400592B8: t_follow_path, 0x400593DC: t_get_ldnumber,
    0x40059520: t_find_volume, 0x40059A88: t_validate, 0x40059AEC: t_f_mount, 0x40059B8C: t_f_open,
    0x40059F7C: t_f_read, 0x4005A1D4: t_f_write, 0x4005A4AC: t_f_sync, 0x4005A694: t_f_close,
    0x4005A6CC: t_f_lseek, 0x4005AA2C: t_f_opendir, 0x4005AB20: t_f_closedir, 0x4005AB48: t_f_readdir,
    0x4005ABC0: t_f_getfree, 0x4005AD8C: t_f_unlink, 0x4005AED4: t_f_mkdir, 0x4005B130: t_f_chmod,
    0x4005B1F8: t_f_mkfs,
}
