/*
 * serializer_encode_scalar_be -- stock 0x40067160 (280 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Stores a 1/2/4/8-byte value big-endian at image+off (image = SYS_INFO+0x360404); other sizes do nothing. The source is re-read for every byte, as in the stock code. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void serializer_encode_scalar_be(u32 off, const volatile void *src, u32 size)
{
    volatile unsigned char *d = SER_IMAGE + off;
    if (size == 1) {
        d[0] = *(const volatile unsigned char *)src;
    } else if (size == 2) {
        const volatile unsigned short *p = src;
        d[0] = *p >> 8;
        d[1] = *p;
    } else if (size == 4) {
        const volatile u32 *p = src;
        d[0] = *p >> 24;
        d[1] = *p >> 16;
        d[2] = *p >> 8;
        d[3] = *p;
    } else if (size == 8) {
        const volatile unsigned long long *p = src;
        d[0] = *p >> 56;
        d[1] = *p >> 48;
        d[2] = *p >> 40;
        d[3] = *p >> 32;
        d[4] = *p >> 24;
        d[5] = *p >> 16;
        d[6] = *p >> 8;
        d[7] = *p;
    }
}
