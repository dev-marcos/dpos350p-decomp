/*
 * __aeabi_memclr4 -- stock 0x40068E5C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: "movs r2, #0; b __aeabi_memset4".
 */
#include "rt/bits.h"

AEABI void __aeabi_memset4(void *dest, u32 n, u32 c);

AEABI void __aeabi_memclr4(void *dest, u32 n)
{
    __aeabi_memset4(dest, n, 0);
}
