/*
 * levels_view_layout -- stock 0x400612FC (632 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, lays out the level view in block 0x42E68668 from its bytes +1 / +2 (the two side panels): the plot box +0xB0 / +0xB4 / +0xB8 (width, also halved into +0xBC) / +0xC0 .. +0xD0 and the panel boxes +0xD4 .. +0xE0 / +0xE4 .. +0xF0. The FFT size +0xF8 becomes 0x1000 / 0x2000 / 0x4000 / 0x8000 for mode +0xF4 = 0..3 (kept otherwise), the sample pointer 0x42E68CB0 = 0x42CC64B4 + 2 x size, SYS+0x3C = 0, SYS+0x54E = 1. From the s64 sample rate SYS+0x538: bin width +0x100 = rate / 1000 / size, top +0x108 = (rate / 1000, unsigned) / 2, centre +0x118 = (top + bin) / 2, span +0x110 = top - bin.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"
#include "state/levels_block.h"
#include "shared/access.h"

#define SYS 0x40544E68u

void levels_view_layout(void)
{
    u32 n, f1, f2, x, y, w, c0, c4, c8, cc, d0;
    long long rate, bin, top;
    if (g_touch->serviced)
        return;
    n = LEVELS(fft_points);
    switch (LEVELS(spur_mode)) {
    case 0: n = 0x1000; break;
    case 1: n = 0x2000; break;
    case 2: n = 0x4000; break;
    case 3: n = 0x8000; break;
    }
    f1 = LEVELS(panel_a_on);
    x = LEVELS(plot_x);
    y = LEVELS(plot_y);
    w = LEVELS(plot_w);
    c0 = LEVELS(plot_h);
    c4 = LEVELS(grid_x_step);
    c8 = LEVELS(grid_x_div);
    cc = LEVELS(grid_y_step);
    d0 = LEVELS(grid_y_div);
    f2 = LEVELS(panel_b_on);
    if (!f1 && !f2) {
        x = 0x34; y = 0x30; w = 0x3C6; c0 = 0x1E0; c4 = 0xA1; c8 = 6; cc = 0x50; d0 = 6;
    } else if (f1) {
        x = 0x34; y = 0x30; w = 0x3C6;
        if (!f2) {
            cc = 0x2F;
            LEVELS(panel_a_x) = 0x34;
            c0 = 0x11A; c4 = 0xA1; c8 = 6; d0 = 6;
            LEVELS(panel_a_y) = 0x163;
            LEVELS(f_0dc) = 0x3C6;
            LEVELS(panel_a_rows) = 0xAB;
        } else {
            LEVELS(panel_a_x) = 0x34;
            c0 = 0x8F;
            LEVELS(panel_b_x) = 0x34;
            LEVELS(panel_a_y) = 0xD8;
            c4 = 0xA1;
            LEVELS(f_0dc) = 0x3C6;
            c8 = 6;
            LEVELS(panel_a_rows) = 0x8F;
            cc = 0x18; d0 = 6;
            LEVELS(panel_b_y) = 0x180;
            LEVELS(f_0ec) = 0x3C6;
            LEVELS(panel_b_rows) = 0x8F;
        }
    } else {
        cc = 0x2F; w = 0x3C6; x = 0x34; y = 0x30; c0 = 0x11A;
        LEVELS(panel_b_x) = 0x34;
        c4 = 0xA1; c8 = 6; d0 = 6;
        LEVELS(panel_b_y) = 0x163;
        LEVELS(f_0ec) = 0x3C6;
        LEVELS(panel_b_rows) = 0xAB;
    }
    LEVELS(grid_y_step) = cc;
    LEVELS(grid_y_div) = d0;
    LEVELS(plot_x) = x;
    LEVELS(plot_y) = y;
    LEVELS(plot_h) = c0;
    LEVELS(grid_x_step) = c4;
    LEVELS(grid_x_div) = c8;
    SI(acq_transition_flag) = 0;
    SI(f_54e) = 1;
    W32(0x42E68CB0u) = 0x42CC64B4u + n * 2;
    LEVELS(fft_points) = n;
    LEVELS(plot_w) = w;
    LEVELS(f_0bc) = (u32)((int)w / 2);
    rate = S64(SYS + 0x538);
    bin = __aeabi_ldivmod(__aeabi_ldivmod(rate, 1000), (long long)(int)LEVELS(fft_points));
    top = (long long)(__aeabi_uldivmod((unsigned long long)rate, 1000) >> 1);
    LEVELS(f_108) = top;
    LEVELS(bin_width) = bin;
    LEVELS(f_118) = __aeabi_ldivmod(top + bin, 2);
    LEVELS(f_110) = LEVELS(f_108) - LEVELS(bin_width);
}
