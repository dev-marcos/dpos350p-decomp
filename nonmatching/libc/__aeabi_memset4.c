/*
 * __aeabi_memset4 -- stock 0x4006A6FC (56 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: fill a word-aligned block (dest, n, c). The stock code stores the 32-bit
 * value in r2 as-is (no byte replication): 16-byte blocks, then an 8-, 4-, 2- and 1-byte
 * tail selected by the low bits of n (halfword = c & 0xFFFF, byte = c & 0xFF).
 */
#include "rt/bits.h"

AEABI void __aeabi_memset4(void *dest, u32 n, u32 c)
{
    u32 *w = dest;
    unsigned short *h;
    unsigned char *b;

    while (n >= 16) { w[0] = c; w[1] = c; w[2] = c; w[3] = c; w += 4; n -= 16; }
    if (n & 8) { w[0] = c; w[1] = c; w += 2; }
    if (n & 4) { *w++ = c; }
    h = (unsigned short *)w;
    if (n & 2) { *h++ = (unsigned short)c; }
    b = (unsigned char *)h;
    if (n & 1) { *b = (unsigned char)c; }
}
