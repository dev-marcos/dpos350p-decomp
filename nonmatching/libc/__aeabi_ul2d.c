/*
 * __aeabi_ul2d -- stock 0x4005F730 (70 bytes, Thumb)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: uint64 (r0:r1) -> double (r0:r1), round to nearest even.
 */
#include "rt/l2d.h"
#include "libc.h"

AEABI u64 __aeabi_ul2d(u64 x)
{
    return u64_to_dbits(x, 0);
}
