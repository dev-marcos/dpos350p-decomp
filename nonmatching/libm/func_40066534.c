/*
 * sin -- stock 0x40066534 (4 bytes, Thumb, hard-float)
 * Status: NONMATCHING; behaviour verified bit-exactly by tools/emu (docs/PROGRESS.md)
 * Arm C library: "movs r0, #0; b __sincos_core" (DERIVED by emulation against math.sin).
 */
double __sincos_core(double x, int q);

double sin(double x)
{
    return __sincos_core(x, 0);
}
