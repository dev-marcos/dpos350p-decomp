/*
 * __aeabi_llsl -- stock 0x4006A530 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 64-bit shift left (shift taken mod 256; ARM register-shift semantics).
 */
#include "hw/platform.h"

unsigned long long __aeabi_llsl(unsigned long long v, int sh)
{
    u32 lo = (u32)v, hi = (u32)(v >> 32), n = (u32)sh & 0xFF, s;
    if (n >= 32) {
        s = n - 32;
        hi = s < 32 ? lo << s : 0;
    } else {
        hi = (hi << n) | (n ? lo >> (32 - n) : 0);
    }
    lo = n < 32 ? lo << n : 0;
    return (unsigned long long)hi << 32 | lo;
}
