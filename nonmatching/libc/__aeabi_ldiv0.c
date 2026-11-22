#include "libc.h"
/*
 * __aeabi_ldiv0 -- stock 0x4003E704 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: 64-bit division-by-zero handler. The stock one just returns ("bx lr"),
 * leaving r0-r3 untouched, so __aeabi_uldivmod(n, 0) returns quotient n, remainder 0.
 */
__attribute__((naked)) void __aeabi_ldiv0(void)
{
    __asm__ volatile("bx lr");
}
