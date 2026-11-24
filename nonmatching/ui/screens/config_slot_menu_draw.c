/*
 * config_slot_menu_draw -- stock 0x40028AB4 (1376 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the list of the five configuration slots (layout +0x76..+0x7C) beside the configuration menu: the box with a black 2-pixel left edge, then five rows of 0x3C with image 0x4008B738, text entry 86 + row ('Configuration 1' .. '5') and touch button 0x42E61FF8 + 0x124 + 0xA x row. A row is shaded with theme colour +8 while pressed, or when the menu selection (layout +0x67) is 3 ('Power-on configuration') and it is the power-on slot SYS+0x7C0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "state/ui_windows.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */

void config_slot_menu_draw(void)
{
    u32 x = WREC(8, x), y = WREC(8, y), w = WREC(8, w), h = WREC(8, h);
    u32 x2, w2, ys, e;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    x2 = x + 2;
    w2 = w - 2;
    for (i = 0; i < 5; i++) {
        ys = (y + 0x14 + 0x3C * i) & 0xFFFF;
        e = TXT + 0x1A0 * (86 + i);
        draw_colour_set(W32(TH + 4));
        draw_text_shadowed((x2 + 0x14) & 0xFFFF, ys, 0x4008B738u, 2, 2, 0);
        draw_colour_set(W32(TH + 4));
        text_shadow_draw((x2 + 0x41 + SB8(e + 0x194)) & 0xFFFF, (ys + 3 + SB8(e + 0x195)) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        touch_button_set_rect(x2 & 0xFFFF, (ys - 0x14) & 0xFFFF, w2 & 0xFFFF, 0x3C, (volatile unsigned char *)(BTN + 0x124 + 0xA * i));
        if (B8(BTN + 0x125 + 0xA * i) || (SI(f_7c0) == i + 1 && WREC(7, f_5) == 3)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(x2 & 0xFFFF, (ys - 0x14) & 0xFFFF, w2 & 0xFFFF, 0x3C);
        }
    }
}
