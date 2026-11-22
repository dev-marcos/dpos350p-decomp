/*
 * main_toolbar_draw -- stock 0x40019DB8 (2304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the bottom toolbar of the oscilloscope view (layout +0x14 / +0x16): a black 0x3BA x 0x2C strip, then nine buttons 0x66 x 0x2C, 0x6A apart, labelled with text entries 0 .. 8 ('System', 'Function', 'Measure', 'Browser', 'Cursor X', 'Cursor Y', 'Save wave', 'Screenshot', 'Zoom'). Each has the toolbar theme background SYS+0x3DD530, an icon 0x40088208 + 0x170 x i at x + 0x2A (its own colour is set and at once replaced by theme +4, as in the other toolbars), the label centred in theme colour +8 and the touch record 0x42E61FF8 + 2 + 0xA x i; a pressed button is filled with theme +0x1C. The last button's background and shade are 0x67 wide.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"

#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed, ... */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

#define TT (SYS + 0x3DD530)          /* toolbar theme: +0 fill, +4 icon, +8 text, +0x1C pressed */

static u32 icon_colour(int i)
{
    switch (i) {
    case 0: case 6: return 0xAA8855;
    case 1: case 7: return 0x5588AA;
    case 2: return 0x88AA55;
    case 3: case 8: return 0x55AA88;
    case 4: return 0xAA5588;
    default: return 0x8855AA;
    }
}

void main_toolbar_draw(void)
{
    u32 x = H16(LAY + 0x14), y = H16(LAY + 0x16), bx, e, tw, wd;
    int i;

    draw_colour_set(0xFFFFFF);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(0);
    rect_fill(x, y, 0x3BA, 0x2C);
    for (i = 0; i < 9; i++) {
        bx = x + 0x6A * i;
        wd = i == 8 ? 0x67 : 0x66;
        e = ENT(i);
        draw_colour_set(W32(TT));
        rect_fill(bx & 0xFFFF, y, wd, 0x2C);
        draw_colour_set(icon_colour(i));
        draw_colour_set(W32(TT + 4));
        draw_text_shadowed((bx + 0x2A) & 0xFFFF, (y + 2) & 0xFFFF, 0x40088208u + 0x170 * i, 2, 2, 0);
        draw_colour_set(W32(TT + 8));
        tw = text_width((const char *)W32(e));
        text_shadow_draw((bx + ((0x66 - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 0x19) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        touch_button_set_rect(bx & 0xFFFF, y, 0x66, 0x2C, (volatile unsigned char *)(BTN + 2 + 0xA * i));
        if (B8(BTN + 3 + 0xA * i)) {
            draw_colour_set(W32(TT + 0x1C));
            rect_fill(bx & 0xFFFF, y, wd, 0x2C);
        }
    }
}
