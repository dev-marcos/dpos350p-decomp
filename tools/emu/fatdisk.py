"""RAM disk + FatFs environment for behavioural tests of the FatFs layer.

* RamDisk: copy-on-write sector store backing the stock diskio entry points.
* stubs(): disk_initialize/read/write/ioctl, get_fattime. Calls are recorded as
  (name, sector, count, sha1(data)) -- never raw buffer pointers, which legitimately differ
  between the stock function and the C build (stack buffers).
* Volumes are formatted by the *stock* f_mkfs and populated through the *stock* API
  (f_mount/f_open/f_write/f_mkdir...), so test fixtures never depend on code under test.
"""
import hashlib
import random
from pathlib import Path

from tools.emu.emu import Machine, Stub, ROOT

SS = 512
# stock entry points (symbols/manual.csv)
A = {
    "disk_initialize": 0x40049C38, "disk_read": 0x40049C78, "disk_write": 0x40049CE8,
    "disk_ioctl": 0x40049D68, "get_fattime": 0x40049DD8,
    "f_mount": 0x40059AEC, "f_open": 0x40059B8C, "f_read": 0x40059F7C, "f_write": 0x4005A1D4,
    "f_sync": 0x4005A4AC, "f_close": 0x4005A694, "f_lseek": 0x4005A6CC, "f_opendir": 0x4005AA2C,
    "f_closedir": 0x4005AB20, "f_readdir": 0x4005AB48, "f_getfree": 0x4005ABC0,
    "f_unlink": 0x4005AD8C, "f_mkdir": 0x4005AED4, "f_chmod": 0x4005B130, "f_mkfs": 0x4005B1F8,
}
# struct layout (R0.13b + stock config, computed with offsetof; see docs/CONTRIBUTING.md)
SZ = {"FATFS": 0x238, "FIL": 0x250, "DIR": 0x50, "FILINFO": 0x120}
FIL_OBJ, FIL_FLAG, FIL_FPTR = 0x0, 0x30, 0x38
DIR_FN, FI_FNAME = 0x40, 0x1A
FATTIME = ((2025 - 1980) << 25) | (6 << 21) | (19 << 16) | (12 << 11)

# fixed work areas in the stock ZI region
FS = 0x41000000          # FATFS
FIL1 = 0x41001000        # FIL objects
FIL2 = 0x41002000
DIR1 = 0x41003000        # DIR objects
FINFO = 0x41004000       # FILINFO
STR = 0x41005000         # path strings
DATA = 0x41010000        # 64 KiB data buffer for f_read/f_write
WORK = 0x41020000        # f_mkfs work area (32 KiB)
SCR = 0x41030000         # scratch for internal-function tests

FM_FAT, FM_FAT32, FM_EXFAT, FM_ANY, FM_SFD = 1, 2, 4, 7, 8
FA_READ, FA_WRITE, FA_OPEN_EXISTING, FA_CREATE_NEW, FA_CREATE_ALWAYS, FA_OPEN_ALWAYS, FA_OPEN_APPEND = \
    1, 2, 0, 4, 8, 0x10, 0x30


class RamDisk:
    def __init__(self, nsect, base=None):
        self.nsect = nsect
        self.base = base            # bytes of a formatted image (shared, never modified)
        self.over = {}              # sector -> bytes (copy-on-write)

    def read(self, s):
        if s in self.over:
            return self.over[s]
        if self.base is None:
            return bytes(SS)
        return self.base[s * SS:(s + 1) * SS]

    def write(self, s, b):
        self.over[s] = bytes(b)

    def image(self):
        img = bytearray(self.base if self.base is not None else bytes(self.nsect * SS))
        for s, b in self.over.items():
            img[s * SS:(s + 1) * SS] = b
        return bytes(img)

    def fork(self):
        """Independent copy sharing the immutable base."""
        d = RamDisk(self.nsect, self.base)
        d.over = dict(self.over)
        return d


def stubs(disk, fail_read=None, fail_write=None):
    def init(m, a):
        return 0

    def rd(m, a):
        pdrv, buff, sector, count = a
        if fail_read and fail_read(sector):
            m.calls[-1] = ("disk_read", sector, count, "FAIL")
            return 1
        if sector + count > disk.nsect:
            return 4
        for k in range(count):
            m.write(buff + k * SS, disk.read(sector + k))
        m.calls[-1] = ("disk_read", sector, count)
        return 0

    def wr(m, a):
        pdrv, buff, sector, count = a
        data = m.read(buff, count * SS)
        m.calls[-1] = ("disk_write", sector, count, hashlib.sha1(data).hexdigest()[:12])
        if fail_write and fail_write(sector):
            return 1
        if sector + count > disk.nsect:
            return 4
        for k in range(count):
            disk.write(sector + k, data[k * SS:(k + 1) * SS])
        return 0

    def ioctl(m, a):
        pdrv, cmd, buff = a[0], a[1], a[2]
        m.calls[-1] = ("disk_ioctl", cmd)
        if cmd == 0:                 # CTRL_SYNC
            return 0
        if cmd == 1:                 # GET_SECTOR_COUNT
            m.w32(buff, disk.nsect); return 0
        if cmd == 2:                 # GET_SECTOR_SIZE
            m.uc.mem_write(buff, SS.to_bytes(2, "little")); return 0
        if cmd == 3:                 # GET_BLOCK_SIZE
            m.w32(buff, 1); return 0
        return 4                     # RES_PARERR

    def fattime(m, a):
        m.calls[-1] = ("get_fattime",)
        return FATTIME

    return [Stub("disk_initialize", A["disk_initialize"], init, 1),
            Stub("disk_read", A["disk_read"], rd), Stub("disk_write", A["disk_write"], wr),
            Stub("disk_ioctl", A["disk_ioctl"], ioctl, 3), Stub("get_fattime", A["get_fattime"], fattime, 0)]


def cstr(m, addr, s):
    b = s.encode("latin-1") + b"\0"
    m.write(addr, b)
    return addr


class Env:
    """A machine with a RAM disk and helpers that drive the *stock* FatFs API."""
    def __init__(self, disk, code=None, extra_stubs=(), max_insns=50_000_000, **stubargs):
        self.disk = disk
        self.m = Machine(code=code, stubs=stubs(disk, **stubargs) + list(extra_stubs), max_insns=max_insns)

    def stock(self, name, *args, stack=()):
        r = self.m.call(A[name], args, stack)
        if r["error"]:
            raise RuntimeError(f"stock {name}: {r['error']}")
        return r["r0"]

    def mount(self):
        cstr(self.m, STR, "0:")
        return self.stock("f_mount", FS, STR, 1)

    def mkfs(self, opt, au=0):
        cstr(self.m, STR, "0:")
        return self.stock("f_mkfs", STR, opt, au, WORK, stack=(0x8000,))

    def open(self, fil, path, mode):
        cstr(self.m, STR, path)
        return self.stock("f_open", fil, STR, mode)

    def write_file(self, path, data, mode=FA_CREATE_ALWAYS | FA_WRITE):
        r = self.open(FIL1, path, mode)
        if r:
            return r
        pos = 0
        while pos < len(data):
            chunk = data[pos:pos + 0x8000]
            self.m.write(DATA, chunk)
            r = self.stock("f_write", FIL1, DATA, len(chunk), SCR)
            if r:
                return r
            pos += len(chunk)
        return self.stock("f_close", FIL1)

    def mkdir(self, path):
        cstr(self.m, STR, path)
        return self.stock("f_mkdir", STR)

    def unlink(self, path):
        cstr(self.m, STR, path)
        return self.stock("f_unlink", STR)


# ---------------------------------------------------------------- fixture volumes
VOLUMES = {
    # name: (sectors, mkfs opt, au)
    "fat12": (4096, FM_FAT | FM_SFD, 0),            # 2 MiB (stock rejects au=512 here: FR_MKFS_ABORTED)
    "fat16": (65536, FM_FAT | FM_SFD, 2048),        # 32 MiB
    "fat32": (131072, FM_FAT32 | FM_SFD, 512),      # 64 MiB, 1-sector clusters (>= 65526 needed)
    "exfat": (131072, FM_EXFAT | FM_SFD, 4096),     # 64 MiB
}
_CACHE = {}

NAMES = ["README.TXT", "a.bin", "Long File Name With Spaces.dat", "dir1", "dir1/sub", "dir1/sub/deep.txt",
         "dir1/x.c", "UPPER", "mixedCase.Txt", "ünï.txt", "frag1.bin", "frag2.bin", "empty.dat"]


def populate(env, rng):
    """Create a deterministic tree, including fragmented files and LFN entries."""
    env.mkdir("dir1"); env.mkdir("dir1/sub"); env.mkdir("UPPER")
    env.write_file("README.TXT", b"hello DPOS350P\n" * 20)
    env.write_file("a.bin", bytes(rng.getrandbits(8) for _ in range(3000)))
    env.write_file("Long File Name With Spaces.dat", bytes(range(256)) * 10)
    env.write_file("dir1/sub/deep.txt", b"deep" * 300)
    env.write_file("dir1/x.c", b"int main(void){return 0;}\n")
    env.write_file("mixedCase.Txt", b"x" * 700)
    env.write_file("empty.dat", b"")
    # fragmentation: interleave two growing files
    for i in range(6):
        env.write_file("frag1.bin", bytes([i]) * (1024 * (i + 1)), mode=FA_OPEN_APPEND | FA_WRITE)
        env.write_file("frag2.bin", bytes([0x80 | i]) * (700 * (i + 1)), mode=FA_OPEN_APPEND | FA_WRITE)
    env.write_file("tmp_to_delete.bin", b"z" * 5000)
    env.unlink("tmp_to_delete.bin")
    for i in range(40):                       # enough entries to span directory clusters/sectors
        env.write_file(f"dir1/file{i:02d}_with_a_longer_name.log", bytes([i]) * (i * 37))


def volume(name):
    """Formatted + populated image (cached per process). Returns a fresh RamDisk fork."""
    if name not in _CACHE:
        nsect, opt, au = VOLUMES[name]
        cache = ROOT / f"build/emu/disks/{name}.img"
        if cache.exists():
            base = cache.read_bytes()
        else:
            d = RamDisk(nsect)
            env = Env(d, max_insns=2_000_000_000)
            r = env.mkfs(opt, au)
            if r:
                raise RuntimeError(f"stock f_mkfs({name}) = {r}")
            if env.mount():
                raise RuntimeError(f"stock f_mount({name}) failed")
            populate(env, random.Random(name))
            base = d.image()
            cache.parent.mkdir(parents=True, exist_ok=True)
            cache.write_bytes(base)
        _CACHE[name] = RamDisk(nsect, base)
    return _CACHE[name].fork()
