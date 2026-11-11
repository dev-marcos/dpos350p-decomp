/*
 * __aeabi_memcpy4 -- stock 0x4005C340 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: memcpy with both pointers word aligned (16-byte LDM/STM loop, then
 * 8/4/2/1-byte tail). Observable effect: n bytes copied forward.
 */
#include "rt/bits.h"

AEABI void __aeabi_memcpy4(void *dst, const void *src, u32 n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
}
