/*
 * __aeabi_memcpy -- stock 0x4005C31C (36 bytes, ARM; continues into 0x4005C340/0x4005C38C)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library memcpy: byte copy until the source is word aligned, then the aligned
 * (__aeabi_memcpy4) or destination-unaligned path. Observable effect: n bytes copied forward.
 */
#include "rt/bits.h"

AEABI void __aeabi_memcpy(void *dst, const void *src, u32 n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
}
