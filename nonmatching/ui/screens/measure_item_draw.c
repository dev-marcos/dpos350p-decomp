/*
 * measure_item_draw -- stock 0x40030AF8 (1116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * One measurement readout at (x, y) for channel ch (0 = SYS+0x9A8 averages, else SYS+0xA08): the label in grey 0x888888 (1 MAX, 2 MIN, 3 AVG, 4 RMS, 5 VP, 6 VPP, 7 F, 8 T, 9 T+, 10 T-, 11 Dt+, 12 Dt-), then in the channel colour (0xFFFF00 for ch 0, 0x00FFFF otherwise) a sign ('+' / '-' for MAX, MIN, AVG and VP, nothing for 0), the number from value_si_scale in the font 0x42E66718, and the unit (uV / mV / V / kV, mHz .. GHz, pS .. S, %). The x position follows u16 0x42E68BE4 after each text. Kinds outside 1..12 make the stock code use a value it never loads (the caller's r4:r5), so they are not covered. The decimals and prefix start at 0 here; the stock code leaves them uninitialised for values of 1e14 and above.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "ui/screens.h"

#define H16(a) (*(volatile unsigned short *)(a))
#define SYS 0x40544E68u
#define U16(v) ((unsigned short)(v))

void measure_item_draw(u32 x, u32 y, u32 ch, u32 kind)
{
    static const char *const label[12] = {"MAX", "MIN", "AVG", "RMS", "VP", "VPP", "F", "T", "T+", "T-", "Dt+", "Dt-"};
    static const char *const volt[5] = {"uV", "mV", "V", 0, "kV"};
    static const char *const hz[5] = {"mHz", "Hz", "KHz", "MHz", "GHz"};
    static const char *const sec[5] = {"pS", "nS", "uS", "mS", "S"};
    volatile long long *avg = (volatile long long *)(ch == 0 ? SYS + 0x9A8 : SYS + 0xA08);
    char sign[2];
    long long v = 0, out;
    int dec = 0, prefix = 0;
    u32 yy, x2, xn, xu;
    const char *unit = 0;
    sign[1] = 0;
    half_42e68be2_set(2);
    draw_colour_set(0x888888);
    x += 3;
    word_42e68c74_set(0x42E658D8u);
    y += 4;
    if (kind - 1 <= 11) {
        text_draw(U16(x), U16(y), label[kind - 1]);
        v = avg[kind - 1];
    }
    if (kind == 1 || kind == 2 || kind == 3 || kind == 5) {
        if (v == 0) {
            sign[0] = 0;
        } else if (v > 0) {
            sign[0] = '+';
        } else {
            sign[0] = '-';
            v = -v;
        }
    } else {
        sign[0] = 0;
        if (v < 0)
            v = -v;
    }
    value_si_scale(kind, 0, v, &out, &dec, &prefix);
    v = out;
    yy = y - 3;
    draw_colour_set(ch == 0 ? 0xFFFF00u : 0x00FFFFu);
    x2 = H16(0x42E68BE4u) + x + 3;
    text_draw(U16(x2), U16(yy), sign);
    half_42e68be2_set(2);
    xn = H16(0x42E68BE4u) + x2;
    word_42e68c74_set(0x42E66718u);
    number_draw_mode0(U16(xn), U16(yy - 2), (u32)v, (u32)((unsigned long long)v >> 32), (unsigned char)dec);
    xu = H16(0x42E68BE4u) + xn + 3;
    half_42e68be2_set(1);
    draw_colour_set(0x888888);
    word_42e68c74_set(0x42E658D8u);
    if (kind >= 1 && kind <= 6) {
        if ((u32)prefix <= 4)
            unit = volt[prefix];
    } else if (kind == 7) {
        if ((u32)prefix <= 4)
            unit = hz[prefix];
    } else if (kind >= 8 && kind <= 10) {
        if ((u32)prefix <= 4)
            unit = sec[prefix];
    } else if (kind == 11 || kind == 12) {
        unit = "%";
    }
    if (unit)
        text_draw(U16(xu), U16(yy - 2 + 5), unit);
}
