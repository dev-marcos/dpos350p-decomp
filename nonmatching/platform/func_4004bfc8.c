/*
 * frontend_offset_calibrate -- stock 0x4004BFC8 (1404 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Offset DAC calibration of volts/div range r (byte argument) for both channels: acq_defaults_reset, both channels on at range r (SYS+0x41 / +0x105) with the other channel fields cleared, SYS+0x588 = 0x18, SYS+0x3A = 1, touch byte 0x42E68BB0 + 0x16 = 0, outputs_apply, acquisition_dispatcher. Then two binary searches over the DAC range half SYS+0x1B6 .. SYS+0x1B4 (at most k steps, k = number of doublings of 1 up to the range width), CH1 through half SYS+0x44 + fpga_write_sel_0x2a, CH2 through SYS+0x108 + fpga_write_sel_0x2d, 100 ms settling, level = channel_level_mean(1, ch, 2000, 0x429D3AB4) / word SYS+0x26C: first for a level in 420 .. 480, then in 20 .. 80 (SYS+0x1B2 flips the search direction). The two points give per channel gain = (dac_hi - dac_lo) x SYS+0x270 x SYS+0x26C / (mean_hi - mean_lo) (64-bit) and offset = SYS+0x270 x dac_lo - mean_lo x gain / SYS+0x26C, stored at 0x42E53A0C + 4r + 0x50 / + 0xA0 (gain CH1 / CH2) and + 0xF0 / + 0x140 (offset).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define TCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S8(a) (*(volatile signed char *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define K 0x429D3AB4u
#define DIV W32(SYS + 0x26C)
#define FLIP B8(SYS + 0x1B2)

void acq_defaults_reset(void);
void fpga_write_sel_0x2a(void);
void fpga_write_sel_0x2d(void);
void fpga_channel_mode_apply(void);
u32 channel_level_mean(u32 a, u32 ch, u32 n, u32 p);

static u32 mean(u32 ch)
{
    B8(SYS + 0x100) = ch + 1;
    fpga_channel_mode_apply();
    return channel_level_mean(1, ch, 2000, K);
}

/* one step of the search for channel ch: narrows lo / hi around mid, returns 1 when the level is inside lo_ok .. hi_ok */
static int step(u32 ch, int mid, int *lo, int *hi, int lo_ok, int hi_ok)
{
    int v = (int)(mean(ch) / DIV);

    if (v < lo_ok) {
        if (FLIP)
            *hi = mid;
        else
            *lo = mid;
        return 0;
    }
    if (v <= hi_ok)
        return 1;
    if (FLIP)
        *lo = mid;
    else
        *hi = mid;
    return 0;
}

static void search(int k, int lo_ok, int hi_ok)
{
    int lo1 = H16(SYS + 0x1B6), hi1 = H16(SYS + 0x1B4), lo2 = lo1, hi2 = hi1, m1 = 0, m2 = 0, d1 = 0, d2 = 0, it = 0;

    for (;;) {
        if (!d1) {
            m1 = (lo1 + hi1) / 2;
            H16(SYS + 0x44) = (unsigned short)m1;
            fpga_write_sel_0x2a();
        }
        if (!d2) {
            m2 = (lo2 + hi2) / 2;
            H16(SYS + 0x108) = (unsigned short)m2;
            fpga_write_sel_0x2d();
        }
        delay_ms(100);
        if (!d1)
            d1 = step(0, m1, &lo1, &hi1, lo_ok, hi_ok);
        if (!d2)
            d2 = step(1, m2, &lo2, &hi2, lo_ok, hi_ok);
        if (d1 && d2)
            return;
        if (++it >= k)
            return;
    }
}

static int gain(u32 dh, u32 dl, u32 vh, u32 vl)
{
    unsigned long long num = (unsigned long long)(long long)(int)(dh - dl) * W32(SYS + 0x270) * DIV;

    return (int)__aeabi_ldivmod((long long)num, (long long)(int)(vh - vl));
}

void frontend_offset_calibrate(u32 r)
{
    int k = 0, p = 1, w, g1, g2;
    u32 dh1, dh2, vh1, vh2, dl1, dl2, vl1, vl2, t;

    r &= 0xFF;
    acq_defaults_reset();
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x588) = 0x18;
    B8(SYS + 0x3E) = 1;
    B8(SYS + 0x41) = r;
    B8(SYS + 0x3F) = 0;
    B8(SYS + 0x40) = 0;
    B8(SYS + 0x46) = 0;
    B8(SYS + 0x102) = 1;
    B8(SYS + 0x105) = r;
    B8(SYS + 0x103) = 0;
    B8(SYS + 0x104) = 0;
    B8(SYS + 0x10A) = 0;
    B8(SYS + 0x275) = 0;
    B8(SYS + 0x660) = 0;
    B8(SYS + 0x3A) = 1;
    B8(TCH + 0x16) = 0;
    outputs_apply();
    acquisition_dispatcher();
    w = (int)H16(SYS + 0x1B4) - (int)H16(SYS + 0x1B6);
    do {
        p *= 2;
        k++;
    } while (w >= p);

    search(k, 0x1A4, 0x1E0);
    dh1 = H16(SYS + 0x44);
    dh2 = H16(SYS + 0x108);
    vh1 = mean(0);
    vh2 = mean(1);
    search(k, 0x14, 0x50);
    dl1 = H16(SYS + 0x44);
    dl2 = H16(SYS + 0x108);
    vl1 = mean(0);
    vl2 = mean(1);
    g1 = gain(dh1, dl1, vh1, vl1);
    g2 = gain(dh2, dl2, vh2, vl2);
    t = 0x42E53A0Cu + 4 * r;
    W32(t + 0x50) = (u32)g1;
    W32(t + 0xA0) = (u32)g2;
    W32(t + 0xF0) = W32(SYS + 0x270) * dl1 - (u32)((int)(vl1 * (u32)g1) / (int)DIV);
    W32(t + 0x140) = W32(SYS + 0x270) * dl2 - (u32)((int)(vl2 * (u32)g2) / (int)DIV);
}
