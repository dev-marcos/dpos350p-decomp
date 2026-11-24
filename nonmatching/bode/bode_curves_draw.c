/*
 * bode_curves_draw -- stock 0x40039254 (968 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: draws the gain curve (0xFF8800, pixel rows u16 0x42E180C8+0x8D20[i]) and the phase curve (0x0088FF, +0x9CC0[i]) as vertical segments between neighbouring points for x = 3 .. width - 3, then a legend box (0x55000000) at the top right with "dB" and "deg", and, unless both bytes +0xAD19 / +0xAD1A are set (input A / input B reached the level in the last sweep), a message box centred on the plot with UI text 129 (neither), 130 (only +0xAD1A) or 131 (only +0xAD19), in the colours SYS+0x3DD550 / +0x3DD554, the text moved by the entry's signed bytes +0x194 / +0x195.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "ui/draw.h"
#include "sys_info.h"
#include "state/bode_block.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define BB 0x42E180C8u
#define W(a) (*(volatile u32 *)(a))
#define DD(a) (*(volatile double *)(a))
#define BH(o) H16(BB + (o))
#define BW(o) W(BB + (o))
#define BD(o) DD(BB + (o))
#define U16(v) ((unsigned short)(v))

static void curve(u32 tab, u32 colour)
{
    u32 x0 = BH(0), y0 = BH(2), a, b;
    int i;
    draw_colour_set(colour);
    for (i = 3; i < (int)BH(4) - 2; i++) {
        a = H16(BB + tab + 2 * i);
        b = H16(BB + tab + 2 * i + 2);
        if (a >= b)
            vline_draw(U16(x0 + i), U16(b + y0), U16(a + y0));
        else
            vline_draw(U16(x0 + i), U16(a + y0), U16(b + y0));
    }
}

static void message(u32 e)
{
    volatile unsigned char *t = (volatile unsigned char *)(0x42C2FE74u + e * 0x1A0u);
    u32 tw = text_width(*(const char *volatile *)t), x, y;
    x = BH(0) + ((u32)(BH(4) - tw - 0x28) >> 1);
    y = BH(2) + ((int)BH(6) - 0x28) / 2;
    draw_colour_set(SI(f_3dd550));
    rect_fill(U16(x), U16(y), U16(tw + 0x28), 0x28);
    draw_colour_set(SI(f_3dd554));
    text_draw(U16(x + (signed char)t[0x194] + 0x14), U16(y + (signed char)t[0x195] + 0xD),
              *(const char *volatile *)t);
}

void bode_curves_draw(void)
{
    u32 x, y;
    if (g_touch->serviced)
        return;
    curve(0x8D20, 0xFF8800);
    curve(0x9CC0, 0x88FF);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    x = BH(0) + BH(4) - 0x40;
    y = BH(2) + 2;
    draw_colour_set(0x55000000);
    rect_fill(U16(x), U16(y), 0x3E, 0x30);
    draw_colour_set(0xFF8800);
    hspan_fill(U16(y + 0xC), U16(x + 5), U16(x + 0x14));
    text_draw(U16(x + 0x1E), U16(y + 5), "dB");
    draw_colour_set(0x88FF);
    y += 0x17;
    hspan_fill(U16(y + 0xC), U16(x + 5), U16(x + 0x14));
    text_draw(U16(x + 0x1E), U16(y + 5), "deg");
    if (BODES(f_ad19)) {
        if (BODES(f_ad1a) == 0)
            message(131);
    } else
        message(BODES(f_ad1a) ? 130 : 129);
}
