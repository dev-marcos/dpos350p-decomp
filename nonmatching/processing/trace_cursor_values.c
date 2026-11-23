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
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    sc = BODES(f_8d18);
    w = (double)BODES(f_004);
    base = BODES(f_8d08);
    p1 = pow(10.0, base + (double)BODES(cursor1) / w * sc);
    p2 = pow(10.0, base + (double)BODES(cursor2) / w * sc);
    top = BODES(f_010);
    c1 = BODES(cursor1);
    bot = BODES(f_018);
    h = (double)BODES(f_006);
    c2 = BODES(cursor2);
    BODES(f_ac70) = p1;
    BODES(f_ac78) = p2;
    BODES(f_ac80) = __builtin_fabs(p2 - p1);
    r = top - bot;
    a1 = top - r * (double)TH(0x8D20 + 2 * c1) / h;
    a2 = top - r * (double)TH(0x8D20 + 2 * c2) / h;
    s5 = BODES(f_020) - BODES(f_028);
    b2 = s5 * (double)TH(0x9CC0 + 2 * c2);
    b1 = s5 * (double)TH(0x9CC0 + 2 * c1);
    BODES(f_ac88) = a1;
    BODES(f_ac90) = a2;
    b2 = b2 / h;
    b1 = b1 / h;
    BODES(f_aca8) = BODES(f_020) - b2;
    BODES(f_ac98) = __builtin_fabs(a2 - a1);
    BODES(f_aca0) = BODES(f_020) - b1;
    BODES(f_acb0) = __builtin_fabs(BODES(f_aca8) - BODES(f_aca0));
}
