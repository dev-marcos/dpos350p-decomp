#include "libm.h"
/*
 * cos -- stock 0x40066530 (4 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library: "movs r0, #1; b __sincos_core" (DERIVED by emulation against math.cos).
 */

double cos(double x)
{
    return __sincos_core(x, 1);
}
