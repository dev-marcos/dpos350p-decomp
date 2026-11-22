/*
 * lane_gain_calibrate -- stock 0x4004B0A0 (3864 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Two-point calibration of the 8 ADC lanes. Resets the acquisition defaults, sets the level SYS+0x60/+0x120 to 450 and then 50, and for each row r = SYS+0x6EC .. SYS+0x6ED (written to SYS+0x588) measures the mean of each lane (lane_level_mean over 2000 samples). With K = level x SYS+0x1C0 x SYS+0x26C and S = SYS+0x268, the gain of lane k is (K450 - K50) x S / (H - L) and the offset (K450 x S - gain x H) / SYS+0x26C (64-bit). Pass 1 (SYS+0x102 = 0) writes CAL_OBJ+0x190 (gains, 8 x 50 ints) and +0x7D0 (offsets); pass 2 (SYS+0x102 = 1, lanes 4..7 measured with ch = 1) writes CAL_OBJ+0xE10 and +0x1450.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "system.h"

#define LCAL_BUF ((unsigned char *)0x429D3AB4u)
#define LCAL_ROWS 50

static void lcal_measure(int n, u32 ch_hi, int v[8][LCAL_ROWS])
{
    int i, k;
    for (i = 0; i < n; i++) {
        SYS8(0x588) = (unsigned char)(i + SYS8(0x6EC));
        outputs_apply();
        delay_ms(20);
        v[0][i] = (int)lane_level_mean(1, 0, 0, 2000, LCAL_BUF);
        for (k = 1; k < 8; k++)
            v[k][i] = (int)lane_level_mean(0, k, k >= 4 ? ch_hi : 0, 2000, LCAL_BUF);
    }
}

static u32 lcal_level(u32 pass, u32 level)
{
    SYS8(0x3E) = 1;
    SYS8(0x102) = pass;
    SYS32(0x60) = level;
    SYS32(0x120) = level;
    outputs_apply();
    delay_ms(100);
    return SYS32(0x60) * SYS32(0x1C0) * SYS32(0x26C);
}

static void lcal_fit(int n, u32 hi, u32 lo, int h[8][LCAL_ROWS], int l[8][LCAL_ROWS], volatile int *t)
{
    u32 s = SYS32(0x268);
    int j, k;
    for (j = 0; j < n; j++) {
        long long d = (long long)(int)(hi - lo) * (long long)s;
        long long e;
        u32 y;
        for (k = 0; k < 8; k++)
            t[k * LCAL_ROWS + j] = (int)__aeabi_ldivmod(d, (long long)(h[k][j] - l[k][j]));
        e = (long long)(int)hi * (long long)s;
        y = SYS32(0x26C);
        for (k = 0; k < 8; k++)
            t[8 * LCAL_ROWS + k * LCAL_ROWS + j] =
                (int)__aeabi_ldivmod(e - (long long)t[k * LCAL_ROWS + j] * h[k][j], (long long)y);
    }
}

void lane_gain_calibrate(void)
{
    int h[8][LCAL_ROWS], l[8][LCAL_ROWS];
    u32 hi, lo;
    int n;
    acq_defaults_reset();
    SYS8(0x3C) = 0;
    SYS8(0x41) = 6;
    SYS8(0x105) = 6;
    SYS8(0x40) = 0;
    SYS8(0x3F) = 0;
    SYS8(0x46) = 0;
    SYS8(0x104) = 0;
    SYS8(0x103) = 0;
    SYS8(0x10A) = 0;
    SYS8(0x275) = 0;
    SYS8(0x660) = 0;
    g_touch->enabled = 0;
    SYS8(0x102) = 0;
    SYS8(0x3E) = 1;
    SYS32(0x60) = 450;
    SYS32(0x120) = 450;
    outputs_apply();
    acquisition_dispatcher();
    delay_ms(100);
    hi = SYS32(0x60) * SYS32(0x1C0) * SYS32(0x26C);
    n = (int)SYS8(0x6ED) - (int)SYS8(0x6EC) + 1;
    lcal_measure(n, 0, h);
    lo = lcal_level(0, 50);
    if (n > 0) {
        lcal_measure(n, 0, l);
        lcal_fit(n, hi, lo, h, l, (volatile int *)((u32)CAL_OBJ + 0x190));
    }
    hi = lcal_level(1, 450);
    lcal_measure(n, 1, h);
    lo = lcal_level(1, 50);
    if (n > 0) {
        lcal_measure(n, 1, l);
        lcal_fit(n, hi, lo, h, l, (volatile int *)((u32)CAL_OBJ + 0xE10));
    }
}
