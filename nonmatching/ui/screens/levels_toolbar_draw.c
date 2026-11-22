/*
 * levels_toolbar_draw -- stock 0x4002AE60 (1460 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the bottom toolbar of the level (spectrum) view in the box at layout +0x21A: a black box, then six buttons every 0xAA from x+3 (records 0x42E61FF8 + 0xB56 + 0xA x n): 'Return', 'Function', 'Screenshot', 'Cursor' (text entries 67, 1, 7, 69) with their icons, an unlabelled icon 0x40085AF4 in 0x0088FF, and the run indicator 0x400857FC drawn red (byte 0x42E68668 set) or green. The last button's background and pressed shade are 0xA7 wide, its touch area 0xA6.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"

#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TT (SYS + 0x3DD530)          /* toolbar theme: +0 fill, +4 icon, +8 text, +0x1C pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define BODE 0x42E180C8u             /* frequency-response view block */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

/* one toolbar button: background, icon (colour c is set and at once replaced by the theme
   colour, as in the stock code), centred label of text entry e, touch record, pressed shade */
static void tool_button(u32 tx, u32 y, u32 c, u32 img, u32 e, u32 rec)
{
    u32 tw;

    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(c);
    draw_colour_set(W32(TT + 4));
    draw_text_shadowed((tx + 0x49) & 0xFFFF, (y + 2) & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TT + 8));
    tw = text_width((const char *)W32(e));
    text_shadow_draw((tx + ((0xA6 - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }
}

void levels_toolbar_draw(void)
{
    u32 x = H16(LAY + 0x21A), y = H16(LAY + 0x21C), w = H16(LAY + 0x21E), h = H16(LAY + 0x220);
    u32 tx;

    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    tx = x + 3;
    tool_button(tx, y, 0xAA8855, 0x40089348u, ENT(67), 0xB56);
    tool_button(tx + 0xAA, y, 0x5588AA, 0x40089BE8u, ENT(1), 0xB60);
    tool_button(tx + 0xAA * 2, y, 0x88AA55, 0x40088C18u, ENT(7), 0xB6A);
    tool_button(tx + 0xAA * 3, y, 0x55AA88, 0x400887C8u, ENT(69), 0xB74);

    tx += 0xAA * 4;
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    draw_colour_set(0x88FF);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, 0x40085AF4u, 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xB7E));
    if (B8(BTN + 0xB7F)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA6, 0x2C);
    }

    tx += 0xAA;
    draw_colour_set(W32(TT));
    rect_fill(tx & 0xFFFF, y, 0xA7, 0x2C);
    draw_colour_set(B8(0x42E68668u) ? 0xFF0000 : 0xFF00);
    draw_text_shadowed((tx + 0x47) & 0xFFFF, (y + 8) & 0xFFFF, 0x400857FCu, 2, 2, 0);
    touch_button_set_rect(tx & 0xFFFF, y, 0xA6, 0x2C, (volatile unsigned char *)(BTN + 0xB88));
    if (B8(BTN + 0xB89)) {
        draw_colour_set(W32(TT + 0x1C));
        rect_fill(tx & 0xFFFF, y, 0xA7, 0x2C);
    }
}
