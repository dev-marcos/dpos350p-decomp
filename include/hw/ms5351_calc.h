/* Shared frequency planning of the stock MS5351M clock routines (0x4005F1F0 / 0x4005F38C). */
#ifndef HW_MS5351_CALC_H
#define HW_MS5351_CALC_H
#include "hw/drivers.h"

typedef struct { u32 freq_scaled, rdiv_bits, ms_div, vco, a, b, high; } ms_plan;

static inline void ms5351_plan(u32 freq, ms_plan *p)
{
    u32 vco = 900000000u, rbits = 0, div = 0;
    if (freq > 200000000u) freq = 200000000u;
    else if (freq < 2500) freq = 2500;
    if (freq - 2500 < 1000 || freq - 3500 < 12500) {           /* < 16 kHz */
        if (freq - 2500 < 1000) vco = 600000000u;
        rbits = 0x70; freq <<= 7;
    } else if (freq - 16000 < 16000)   { rbits = 0x60; freq <<= 6; }
    else if (freq - 32000 < 32000)     { rbits = 0x50; freq <<= 5; }
    else if (freq - 64000 < 64000)     { rbits = 0x40; freq <<= 4; }
    else if (freq - 128000 < 128000)   { rbits = 0x30; freq <<= 3; }
    else if (freq - 256000 < 256000)   { rbits = 0x20; freq <<= 2; }
    else if (freq - 512000 < 512000)   { rbits = 0x10; freq <<= 1; }
    p->high = freq - 2500 >= 0x08F0C7BDu;                      /* freq > 150 MHz */
    if (!p->high) {
        div = vco / freq;
        if (div & 1) div -= 1;
        vco = freq * div;
    } else {
        vco = freq << 2;
    }
    if (vco > 900000000u) vco = 900000000u;
    else if (vco < 600000000u) vco = 600000000u;
    u32 a = (u32)(((u64)vco * 0xABCC7712u) >> 32) >> 24;     /* vco / 25 MHz */
    float fr = (float)(vco - 25000000u * a);
    fr = fr * 1048575.0f;
    fr = fr / 25000000.0f;
    p->freq_scaled = freq; p->rdiv_bits = rbits; p->ms_div = div;
    p->vco = vco; p->a = a; p->b = (u32)fr;
}
#endif
