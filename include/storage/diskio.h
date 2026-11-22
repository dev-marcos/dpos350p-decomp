/* Functions of nonmatching/storage/diskio/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef STORAGE_DISKIO_H
#define STORAGE_DISKIO_H

#include "rt/bits.h"

int disk_mount(void);                                                                            /* 0x40049B80 */
int disk_remount(void);                                                                          /* 0x40049B90 */
int disk_format(void);                                                                           /* 0x40049BB4 */
u32 disk_free_bytes(void);                                                                       /* 0x40049BE4 */
int disk_initialize(u32 pdrv);                                                                   /* 0x40049C38 */
int disk_read(u32 pdrv, unsigned char *buff, u32 sector, u32 count);                             /* 0x40049C78 */
int disk_write(u32 pdrv, const unsigned char *buff, u32 sector, u32 count);                      /* 0x40049CE8 */
int disk_ioctl(u32 pdrv, u32 cmd, void *buff);                                                   /* 0x40049D68 */
u32 get_fattime(void);                                                                           /* 0x40049DD8 */
void disk_check_file(void);                                                                      /* 0x40051C28 */

#endif
