/*
 * side_menu_column_draw -- stock 0x4001A708 (1464 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the right-hand menu column (layout +0x22 / +0x24, 0x40 wide, 0x258 high): eight buttons of about 0x4B at y + 0, 0x4B, 0x96, 0xE1, 0x127, 0x175, 0x1BF, 0x20A, each with a background (theme SYS+0x3DD53C, or 0x282828 for buttons 4 .. 6), an icon at x + 0x15 (images 0x400857FC, 0x40085AF4, 0x40085DEC, 0x40086CC4, 0x400866D4 or 0x400869CC, 0x40086FBC, 0x400851D4, 0x400854E8) and a touch record 0x42E61FF8 + 0x5C + 0xA x i; a pressed button (record byte +1 == 1) is filled with theme +0x10. The first icon is red when SYS+0x3C is set and green when not; the fifth uses image 0x400869CC in blue when SYS+0x664 is set, 0x400866D4 in orange when not.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "state/ui_windows.h"
#include "shared/access.h"
#include "state/theme.h"

#define SYS 0x40544E68u
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define ENT(n) (TXT + 0x1A0 * (n))

/* the pressed shade of one button */
static void pressed(u32 x, u32 y, u32 rec, u32 hp)
{
    if (B8(BTN + rec + 1) == 1) {
        draw_colour_set(THEME_PRESSED);
        rect_fill(x, y & 0xFFFF, 0x40, hp);
    }
}

static void icon_rec(u32 x, u32 y, u32 yi, u32 img, u32 rec)
{
    draw_text_shadowed((x + 0x13) & 0xFFFF, yi & 0xFFFF, img, 2, 2, 0);
    touch_button_set_rect(x & 0xFFFF, y & 0xFFFF, 0x40, 0x46, (volatile unsigned char *)(BTN + rec));
}

void side_menu_column_draw(void)
{
    u32 x = WREC(2, x) + 2, y = WREC(2, y);

    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x & 0xFFFF, y, 0x40, 0x47);
    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x & 0xFFFF, y, 0x40, 0x258);
    draw_colour_set(SI(acq_transition_flag) ? 0xFF0000 : 0xFF00);
    icon_rec(x, y, y + 0x14, 0x400857FCu, 0x5C);
    pressed(x & 0xFFFF, y, 0x5C, 0x47);

    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x & 0xFFFF, (y + 0x4B) & 0xFFFF, 0x40, 0x47);
    draw_colour_set(0x2288FF);
    draw_colour_set(THEME_WORD(0x10));
    draw_colour_set(0x88FF);
    icon_rec(x, y + 0x4B, y + 0x5F, 0x40085AF4u, 0x66);
    pressed(x & 0xFFFF, y + 0x4B, 0x66, 0x47);

    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x & 0xFFFF, (y + 0x96) & 0xFFFF, 0x40, 0x47);
    draw_colour_set(0x22CC66);
    draw_colour_set(THEME_WORD(0x10));
    icon_rec(x, y + 0x96, y + 0xAA, 0x40085DECu, 0x70);
    pressed(x & 0xFFFF, y + 0x96, 0x70, 0x47);

    draw_colour_set(0x282828);
    rect_fill(x & 0xFFFF, (y + 0xE1) & 0xFFFF, 0x40, 0x46);
    draw_colour_set(0x22CC66);
    draw_colour_set(THEME_WORD(0x10));
    icon_rec(x, y + 0xE1, y + 0xF5, 0x40086CC4u, 0x7A);
    pressed(x & 0xFFFF, y + 0xE1, 0x7A, 0x46);

    draw_colour_set(0x282828);
    rect_fill(x & 0xFFFF, (y + 0x127) & 0xFFFF, 0x40, 0x4E);
    draw_colour_set(0xAAAAAA);
    if (!SI(f_664)) {
        draw_colour_set(0xFFAA33);
        draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x140) & 0xFFFF, 0x400866D4u, 2, 2, 0);
    } else {
        draw_colour_set(0x33AAFF);
        draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x140) & 0xFFFF, 0x400869CCu, 2, 2, 0);
    }
    touch_button_set_rect(x & 0xFFFF, (y + 0x127) & 0xFFFF, 0x40, 0x46, (volatile unsigned char *)(BTN + 0x84));
    pressed(x & 0xFFFF, y + 0x128, 0x84, 0x4B);

    draw_colour_set(0x282828);
    rect_fill(x & 0xFFFF, (y + 0x175) & 0xFFFF, 0x40, 0x46);
    draw_colour_set(0x22CC66);
    draw_colour_set(THEME_WORD(0x10));
    icon_rec(x, y + 0x175, y + 0x189, 0x40086FBCu, 0x8E);
    pressed(x & 0xFFFF, y + 0x174, 0x8E, 0x47);

    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x & 0xFFFF, (y + 0x1BF) & 0xFFFF, 0x40, 0x47);
    draw_colour_set(0x88FF22);
    draw_colour_set(THEME_WORD(0x10));
    icon_rec(x, y + 0x1BF, y + 0x1D6, 0x400851D4u, 0x98);
    pressed(x & 0xFFFF, y + 0x1BF, 0x98, 0x47);

    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x & 0xFFFF, (y + 0x20A) & 0xFFFF, 0x40, 0x48);
    draw_colour_set(0x8822FF);
    draw_colour_set(THEME_WORD(0x10));
    draw_colour_set(0x8800FF);
    icon_rec(x, y + 0x20A, y + 0x221, 0x400854E8u, 0xA2);
    pressed(x & 0xFFFF, y + 0x20A, 0xA2, 0x48);
}
