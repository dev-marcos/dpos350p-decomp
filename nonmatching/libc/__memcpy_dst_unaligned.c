/*
 * __memcpy_dst_unaligned -- stock 0x4005C38C (100 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library internal: tail of __aeabi_memcpy for a word-aligned source and an
 * unaligned destination (word loads, halfword/byte stores). Name is OPEN.
 */
#include "rt/bits.h"
#include "libc.h"

AEABI void __memcpy_dst_unaligned(void *dst, const void *src, u32 n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
}
