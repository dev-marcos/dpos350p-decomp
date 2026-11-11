/*
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
