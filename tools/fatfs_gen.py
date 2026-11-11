#!/usr/bin/env python3
"""Generate the FatFs R0.13b part of the decompilation from the pristine ChaN sources.

Inputs : third_party/fatfs/R0.13b/source/{ff.c,ff.h,integer.h,diskio.h}  (tools/setup/20-third-party.sh)
Outputs: include/fatfs/{ff.h,integer.h,diskio.h}   verbatim (LF line endings)
         include/fatfs/ffconf.h                    stock configuration (evidence inline)
         include/fatfs/ff_priv.h                   ff.c prelude; module statics -> extern at stock
                                                   addresses; prototypes of the internal functions
         nonmatching/fatfs/func_XXXXXXXX.c         one function each, text verbatim from ff.c

The mapping stock address -> FatFs function is DERIVED from source order, call graph and
bodies (docs/COMPILER.md, docs/MEMORY_MAP.md). Re-running overwrites the generated files.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "third_party/fatfs/R0.13b/source"

# stock address -> ff.c function (build 2025-06-19)
MAP = {
    0x40056A70: "ld_word", 0x40056A80: "ld_dword", 0x40056AA0: "ld_qword",
    0x40056B04: "st_word", 0x40056B14: "st_dword", 0x40056B34: "st_qword",
    0x40056B90: "mem_cpy", 0x40056BAC: "mem_set", 0x40056BBC: "mem_cmp", 0x40056BE0: "chk_chr",
    0x40056C00: "dbc_1st", 0x40056C08: "dbc_2nd", 0x40056C10: "tchar2uni", 0x40056C84: "put_utf",
    0x40056CF0: "sync_window", 0x40056D80: "move_window", 0x40056DD8: "sync_fs",
    0x40056EA8: "clst2sect", 0x40056ED4: "get_fat", 0x40057108: "put_fat",
    0x4005728C: "find_bitmap", 0x40057374: "change_bitmap", 0x40057428: "fill_first_frag",
    0x40057480: "fill_last_frag", 0x400574D4: "remove_chain", 0x400576B0: "create_chain",
    0x4005793C: "dir_clear", 0x400579D4: "dir_sdi", 0x40057AEC: "dir_next", 0x40057C38: "dir_alloc",
    0x40057CDC: "ld_clust", 0x40057D14: "st_clust", 0x40057D50: "cmp_lfn", 0x40057E38: "pick_lfn",
    0x40057EF0: "put_lfn", 0x40057F9C: "gen_numname", 0x40058094: "sum_sfn", 0x400580C4: "xdir_sum",
    0x40058118: "xname_sum", 0x40058170: "xsum32", 0x40058180: "get_xfileinfo",
    0x40058284: "load_xdir", 0x400583E4: "init_alloc_info", 0x40058434: "load_obj_xdir",
    0x4005848C: "store_xdir", 0x4005853C: "create_xdir", 0x4005860C: "dir_read",
    0x400587C8: "dir_find", 0x400589F8: "dir_register", 0x40058CE4: "dir_remove",
    0x40058D7C: "get_fileinfo", 0x40058F8C: "create_name", 0x400592B8: "follow_path",
    0x400593DC: "get_ldnumber", 0x40059454: "check_fs", 0x40059520: "find_volume",
    0x40059A88: "validate", 0x40059AEC: "f_mount", 0x40059B8C: "f_open", 0x40059F7C: "f_read",
    0x4005A1D4: "f_write", 0x4005A4AC: "f_sync", 0x4005A694: "f_close", 0x4005A6CC: "f_lseek",
    0x4005AA2C: "f_opendir", 0x4005AB20: "f_closedir", 0x4005AB48: "f_readdir",
    0x4005ABC0: "f_getfree", 0x4005AD8C: "f_unlink", 0x4005AED4: "f_mkdir", 0x4005B130: "f_chmod",
    0x4005B1F8: "f_mkfs",
}

# module statics of ff.c that live in the stock ZI region (found in find_volume/f_mount pools)
STATICS = {
    "static FATFS* FatFs[FF_VOLUMES];": "extern FATFS* FatFs[FF_VOLUMES];   /* stock 0x42E68C6C */",
    "static WORD Fsid;": "extern WORD Fsid;                  /* stock 0x42E68D14 */",
    "static BYTE\tDirBuf[MAXDIRB(FF_MAX_LFN)];": "extern BYTE DirBuf[MAXDIRB(FF_MAX_LFN)];  /* stock 0x42E67BC4 */",
    "static WCHAR LfnBuf[FF_MAX_LFN + 1];": "extern WCHAR LfnBuf[FF_MAX_LFN + 1];     /* stock 0x42E6805C */",
}

FFCONF = r'''/*
 * FatFs R0.13b configuration of the DPOS350P stock firmware (build 2025-06-19).
 * Evidence (docs/COMPILER.md, docs/MEMORY_MAP.md) in brackets; OPEN values are guesses
 * that do not change any code that exists in the stock binary.
 */
#define FFCONF_DEF 63463   /* R0.13b revision ID */

#define FF_FS_READONLY   0   /* [DERIVED] f_write/f_mkfs present */
#define FF_FS_MINIMIZE   0   /* [DERIVED] f_getfree/f_unlink/f_mkdir present (f_stat/f_truncate/f_rename removed by FNIRSI) */
#define FF_USE_STRFUNC   0   /* [DERIVED] no f_gets/f_putc after f_mkfs */
#define FF_USE_FIND      0   /* [DERIVED] no pattern_matching/f_findfirst */
#define FF_USE_MKFS      1   /* [DERIVED] f_mkfs 0x4005B1F8 */
#define FF_USE_FASTSEEK  0   /* [DERIVED] no clmt_clust */
#define FF_USE_EXPAND    0   /* [DERIVED] no f_expand */
#define FF_USE_CHMOD     1   /* [DERIVED] f_chmod 0x4005B130 (f_utime removed by FNIRSI) */
#define FF_USE_LABEL     1   /* [DERIVED] dir_read(dp, vol=1) scans for the exFAT label (f_getlabel/f_setlabel removed by FNIRSI) */
#define FF_USE_FORWARD   0   /* [DERIVED] no f_forward */
#define FF_CODE_PAGE     437 /* [DERIVED] tchar2uni/put_utf pass 0x1B5 */
#define FF_USE_LFN       1   /* [DERIVED] find_volume stores static LfnBuf/DirBuf */
#define FF_MAX_LFN       255 /* [DERIVED] DirBuf..LfnBuf sizes */
#define FF_LFN_UNICODE   0   /* [DERIVED] ANSI/OEM API (ff_oem2uni/ff_uni2oem) */
#define FF_LFN_BUF       255 /* [OPEN] default */
#define FF_SFN_BUF       12  /* [OPEN] default */
#define FF_STRF_ENCODE   3   /* unused (no string functions) */
#define FF_FS_RPATH      0   /* [DERIVED] FATFS layout, no f_chdir */
#define FF_VOLUMES       1   /* [DERIVED] get_ldnumber accepts only drive 0 */
#define FF_STR_VOLUME_ID 0   /* [DERIVED] get_ldnumber parses digits only */
#define FF_VOLUME_STRS   "RAM","NAND","CF","SD","SD2","USB","USB2","USB3"
#define FF_MULTI_PARTITION 0 /* [DERIVED] no f_fdisk */
#define FF_MIN_SS        512 /* [DERIVED] FATFS layout (no ssize) */
#define FF_MAX_SS        512
#define FF_USE_TRIM      0   /* [OPEN] */
#define FF_FS_NOFSINFO   0   /* [OPEN] default */
#define FF_FS_TINY       0   /* [DERIVED] FATFS has win[], FIL layout (OPEN until checked) */
#define FF_FS_EXFAT      1   /* [DERIVED] exFAT code throughout */
#define FF_FS_NORTC      0   /* [DERIVED] get_fattime 0x40049DD8 is called */
#define FF_NORTC_MON     1
#define FF_NORTC_MDAY    1
#define FF_NORTC_YEAR    2018
#define FF_FS_LOCK       0   /* [DERIVED] no lock functions */
#define FF_FS_REENTRANT  0   /* [DERIVED] FATFS layout, no ff_req_grant */
#define FF_FS_TIMEOUT    1000
#define FF_SYNC_t        HANDLE
'''

HDR = """/*
 * {name} -- stock 0x{addr:08X} ({size} bytes, ARM)
 * Status: NONMATCHING (no licensed armcc); behaviour verified by tools/emu (docs/PROGRESS.md)
 * Generated by tools/fatfs_gen.py: function text verbatim from FatFs R0.13b ff.c (ChaN),
 * built with the stock configuration (include/fatfs/ffconf.h). Licence notice:
 * include/fatfs/ff.h (retained as required by the FatFs licence).
 */
#include "fatfs/ff_priv.h"

"""


def lf(p):
    return p.read_text(encoding="latin-1").replace("\r\n", "\n")


def find_def(src, name):
    """Return (start, end, header_text) of the definition of 'name' in ff.c text."""
    m = re.search(rf"^(?:static\s+)?[A-Za-z_][\w\s\*]*?\b{name}\s*\(", src, re.M)
    while m:
        # skip prototypes: a definition has '{' at column 0 before the next ';' at column 0
        brace = src.find("\n{", m.start())
        semi = src.find(";", m.start())
        if brace != -1 and (semi == -1 or brace < semi or src[m.start():brace].count("(") > 0 and
                            src[m.start():brace].rstrip().endswith(")")):
            end = src.find("\n}\n", brace)
            return m.start(), end + 3, src[m.start():brace]
        m = re.compile(rf"^(?:static\s+)?[A-Za-z_][\w\s\*]*?\b{name}\s*\(", re.M).search(src, m.end())
    raise SystemExit(f"definition of {name} not found")


def prototype(header):
    h = re.sub(r"/\*.*?\*/", "", header, flags=re.S)
    h = re.sub(r"^static\s+", "", h.strip())
    h = re.sub(r"\s+", " ", h).replace("( ", "(").replace(" )", ")")
    return h + ";"


def between_macros(src, first):
    spans = [find_def(src, n)[:2] for n in MAP.values()]
    out = []
    for m in re.finditer(r"^#define .*$", src[first:], re.M):
        p = first + m.start()
        if not any(s <= p < e for s, e in spans):
            out.append(m.group())
    return out


def main():
    inc = ROOT / "include/fatfs"
    inc.mkdir(parents=True, exist_ok=True)
    for f in ("ff.h", "integer.h", "diskio.h"):
        (inc / f).write_text(lf(SRC / f), newline="\n")
    (inc / "ffconf.h").write_text(FFCONF, newline="\n")

    src = lf(SRC / "ff.c")
    first = min(find_def(src, n)[0] for n in MAP.values())
    prelude = src[:first]
    prelude = prelude.replace('#include "ff.h"', '#include "ff.h"\t\t\t/* include/fatfs/ff.h */')
    for old, new in STATICS.items():
        assert old in prelude, old
        prelude = prelude.replace(old, new)
    protos = []
    for addr, name in sorted(MAP.items()):
        s, e, hdr = find_def(src, name)
        if not name.startswith("f_"):
            protos.append(prototype(hdr))
    priv = ["/* Generated by tools/fatfs_gen.py from FatFs R0.13b ff.c (ChaN): the module prelude",
            " * (macros, types, tables) with the module statics bound to their stock addresses,",
            " * plus prototypes of the internal functions (static in ff.c, separate files here). */",
            "#ifndef FF_PRIV_H", "#define FF_PRIV_H", "", prelude.rstrip(), "",
            "/* internal functions (stock addresses in tools/fatfs_gen.py MAP) */", *protos,
            "", "/* macros ff.c defines between functions */", *between_macros(src, first),
            "", "/* Stock diskio differences (DERIVED from the binary):",
            " * disk_status() is inlined as 'return 0' in find_volume (0x40059570: ldrb pdrv; mov r0,#0),",
            " * i.e. the FNIRSI diskio layer reports a ready drive without a call. */",
            "#define disk_status(pdrv) ((void)(pdrv), (DSTATUS)0)",
            "DWORD get_fattime(void);   /* stock 0x40049DD8 */", "", "#endif /* FF_PRIV_H */", ""]
    (inc / "ff_priv.h").write_text("\n".join(priv), newline="\n")

    out = ROOT / "nonmatching/fatfs"
    out.mkdir(parents=True, exist_ok=True)
    import json
    m = json.loads((ROOT / "build/analysis/map.json").read_text())
    size = {f["start"]: f["end"] - f["start"] for f in m["functions"]}
    for addr, name in sorted(MAP.items()):
        s, e, hdr = find_def(src, name)
        body = re.sub(r"^static\s+", "", src[s:e])
        (out / f"func_{addr:08x}.c").write_text(HDR.format(name=name, addr=addr, size=size.get(addr, 0)) + body,
                                                 newline="\n")
    print(f"generated {len(MAP)} functions, ff_priv.h with {len(protos)} prototypes")


if __name__ == "__main__":
    main()
