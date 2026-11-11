# FatFs probe configuration

`ffconf.h` is FatFs R0.13b's default configuration with only the options that the stock
firmware provably uses changed (evidence in docs/COMPILER.md):

- `FF_USE_MKFS 1`: `f_mkfs` is present (0x4005B1F8)
- `FF_FS_EXFAT 1`: exFAT boot signature in `check_fs`/`f_mkfs`
- `FF_USE_LFN 2`: required by exFAT. The storage mode (1/2/3) is not yet determined (OPEN)
- `FF_CODE_PAGE 437`: tchar2uni/put_utf pass cp=0x1B5 (437) to ff_oem2uni/ff_uni2oem (DERIVED)

FatFs sources are read from `third_party/fatfs/R0.13b/source` (BSD-style licence, ChaN).
