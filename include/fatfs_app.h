/* FatFs API as the application calls it: FIL, DIR and FATFS are passed as raw buffers. FatFs itself uses include/fatfs/ff.h.
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef FATFS_APP_H
#define FATFS_APP_H

#include "rt/bits.h"

int f_mount(void *, const char *, int);                                                          /* 0x40059AEC */
int f_open(void *, const char *, unsigned char);                                                 /* 0x40059B8C */
int f_read(void *, void *, u32, u32 *);                                                          /* 0x40059F7C */
int f_write(void *, const void *, u32, u32 *);                                                   /* 0x4005A1D4 */
int f_close(void *);                                                                             /* 0x4005A694 */
int f_lseek(void *, unsigned long long);                                                     /* 0x4005A6CC */
int f_opendir(void *, const char *);                                                             /* 0x4005AA2C */
int f_closedir(void *);                                                                          /* 0x4005AB20 */
int f_readdir(void *, void *);                                                                   /* 0x4005AB48 */
int f_getfree(const char *, u32 *, void **);                                                     /* 0x4005ABC0 */
int f_unlink(const char *);                                                                      /* 0x4005AD8C */
int f_mkdir(const char *);                                                                       /* 0x4005AED4 */
int f_chmod(const char *, unsigned char, unsigned char);                                         /* 0x4005B130 */
int f_mkfs(const char *, int, u32, void *, u32);                                                 /* 0x4005B1F8 */

#endif
