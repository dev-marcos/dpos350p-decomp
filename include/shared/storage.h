/* File and settings storage: the path buffer and the settings serializer state
 * (image, mode, cursor) (build 2025-06-19, see docs/DRIVERS.md). */
#ifndef SHARED_STORAGE_H
#define SHARED_STORAGE_H

#define PATHBUF ((const char *)0x4054D728u)                  /* SYS+0x88C0: path built by the list's path helper */
#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_IMAGE ((volatile void *)0x408A526Cu)             /* SYS_INFO+0x360404 */
#define SER_MODE (*(volatile unsigned char *)0x4091F390u)    /* 0 = encode, 1 = decode */

#endif /* SHARED_STORAGE_H */
