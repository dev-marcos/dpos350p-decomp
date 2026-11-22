/*
 * sine_table_build -- stock 0x4005D27C (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * T[i] = (int)(sin(a_i) x 10000 x 10000 / 3.1415926) for i = 0..20000. Stock quirk: a_0 is the caller's d0 (taken as the parameter a0) and a_i = (i - 1) x 0.000314159265359 after that (the index is converted before it is incremented), so T[1] = sin(0) and each entry lags one step.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SINT ((volatile int *)0x42D564B4u)
double sin(double x);

void sine_table_build(double a0)
{
    int i;
    double a = a0;
    for (i = 0; i < 20001; i++) {
        double v = sin(a) * 10000.0;
        SINT[i] = (int)((v * 10000.0) / 3.1415926);
        a = (double)i * 0.000314159265359;
    }
}
