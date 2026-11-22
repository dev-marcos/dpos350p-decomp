/*
 * serializer_decode_scalar_be -- stock 0x400672A8 (376 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads a 1/2/4/8-byte big-endian value from image+off into *dst (the stock code stores each partial result). (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void serializer_decode_scalar_be(u32 off, volatile void *dst, u32 size)
{
    volatile unsigned char *b = SER_IMAGE + off;
    if (size == 1) {
        *(volatile unsigned char *)dst = b[0];
    } else if (size == 2) {
        volatile unsigned short *p = dst;
        u32 v = (u32)b[0] << 8;
        *p = v;
        *p = v | b[1];
    } else if (size == 4) {
        volatile u32 *p = dst;
        u32 v = (u32)b[0] << 8;
        *p = v;
        v = (v | b[1]) << 8;
        *p = v;
        v = (v | b[2]) << 8;
        *p = v;
        *p = v | b[3];
    } else if (size == 8) {
        volatile unsigned long long *p = dst;
        unsigned long long v = (unsigned long long)b[0] << 8;
        int i;
        *p = v;
        for (i = 1; i < 7; i++) {
            v = (v | b[i]) << 8;
            *p = v;
        }
        *p = v | b[7];
    }
}
