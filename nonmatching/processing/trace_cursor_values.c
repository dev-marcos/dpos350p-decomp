/*
 * trace_cursor_values -- stock 0x400381E8 (388 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: converts the two cursor positions (words 0x42E22D2C/30) to values. Horizontally: 10^(base + pos / width * scale) for both, their difference; vertically, from the per-x tables at 0x42E20DE8 (gain) / 0x42E21D88 (phase), the two levels in each of two scales and their differences. Results are doubles at 0x42E22D38..0x42E22D78 (block 0x42E180C8 + 0xAC70..0xACB0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libm.h"
#include "processing.h"
#include "state/bode_block.h"

#define TH(o) (*(volatile unsigned short *)(0x42E180C8u + (o)))

void trace_cursor_values(void)
{
    double sc, w, base, p1, p2, top, bot, h, s5, a1, a2, r, b1, b2;
    int c1, c2;
    if (g_touch->serviced)
        return;
    sc = BODES(log_span);
    w = (double)BODES(plot_w);
    base = BODES(log_start);
    p1 = pow(10.0, base + (double)BODES(cursor1) / w * sc);
    p2 = pow(10.0, base + (double)BODES(cursor2) / w * sc);
    top = BODES(gain_top);
    c1 = BODES(cursor1);
    bot = BODES(gain_bottom);
    h = (double)BODES(plot_h);
    c2 = BODES(cursor2);
    BODES(c1_freq) = p1;
    BODES(c2_freq) = p2;
    BODES(dc_freq) = __builtin_fabs(p2 - p1);
    r = top - bot;
    a1 = top - r * (double)TH(0x8D20 + 2 * c1) / h;
    a2 = top - r * (double)TH(0x8D20 + 2 * c2) / h;
    s5 = BODES(phase_top) - BODES(phase_bottom);
    b2 = s5 * (double)TH(0x9CC0 + 2 * c2);
    b1 = s5 * (double)TH(0x9CC0 + 2 * c1);
    BODES(c1_gain) = a1;
    BODES(c2_gain) = a2;
    b2 = b2 / h;
    b1 = b1 / h;
    BODES(c2_phase) = BODES(phase_top) - b2;
    BODES(dc_gain) = __builtin_fabs(a2 - a1);
    BODES(c1_phase) = BODES(phase_top) - b1;
    BODES(dc_phase) = __builtin_fabs(BODES(c2_phase) - BODES(c1_phase));
}
