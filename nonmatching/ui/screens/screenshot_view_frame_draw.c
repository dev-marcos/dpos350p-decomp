/*
 * screenshot_view_frame_draw -- stock 0x40025DB4 (1324 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the frame of the screenshot viewer window (box 0x42E64520 +0x148..+0x14E): the body becomes touch button 0x42E61FF8+0x80E; a 0x1E-high title bar 0x202020 with a 7-line dark gradient under it, the image 0x4008A038 and text-table entry 110 (the label of the first row of menu_three_rows_draw), and right-aligned at x+0x3ED the file name built by screenshot_path_make from the current entry of the list at SYS+0xFA4; a 0x3C-high bottom bar at y+0x21C with a gradient above it, three separators and four touch buttons (+0x818 / +0x822 / +0x82C / +0x836, shaded with theme colour SYS+0x3DD54C while pressed) labelled with text entries 67, 115, 116, 117 centred in 0xFD. (The separators and labels use 0x21C - y where the bar uses y + 0x21C, as in the stock code.)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "sys_info.h"
#include "state/ui_windows.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define ENT(n) (TXT + 0x1A0 * (n))

void screenshot_view_frame_draw(void)
{
    u32 x = WREC(23, x), y = WREC(23, y);
    u32 w = WREC(23, w), h = WREC(23, h);
    u32 k, e, bx, by, ny, tw;

    touch_button_set_rect(x, (y + 0x1E) & 0xFFFF, w, (h - 0x5A) & 0xFFFF, (volatile unsigned char *)(BTN + 0x80E));
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x202020);
    rect_fill(x, y, w, 0x1E);
    for (k = 1; k < 8; k++) {
        draw_colour_set(k << 29);
        hspan_fill((y + 0x1E + k - 1) & 0xFFFF, x, (w + x) & 0xFFFF);
    }
    draw_colour_set(0xDDDDDD);
    draw_text_shadowed((x + 5) & 0xFFFF, (y + 5) & 0xFFFF, 0x4008A038u, 2, 2, 0);
    e = ENT(110);
    text_shadow_draw((x + SB8(e + 0x194) + 0x23) & 0xFFFF, (y + SB8(e + 0x195) + 8) & 0xFFFF, (const char *)W32(e), 1, 2, 0);

    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x777777);
    screenshot_path_make(H16(SYS + 0xFA6 + 2 * (short)H16(SYS + 0xFA4)));
    tw = text_width((const char *)(SYS + 0x88C1));
    text_shadow_draw((0x3ED + x - tw) & 0xFFFF, (y + 7) & 0xFFFF, (const char *)(SYS + 0x88C1), 2, 2, 0);

    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x202020);
    by = (y + 0x21C) & 0xFFFF;
    rect_fill(x, by, w, 0x3C);
    for (k = 1; k < 8; k++) {
        draw_colour_set(k << 29);
        hspan_fill((by - k) & 0xFFFF, x, (w + x) & 0xFFFF);
    }
    ny = 0x21C - y;
    draw_colour_set(0);
    for (k = 1; k < 4; k++)
        rect_fill((x + 0xFF * k) & 0xFFFF, (ny + 0xF) & 0xFFFF, 2, 0x1E);
    for (k = 0; k < 4; k++) {
        bx = k ? x + 0xFF * k + 2 : x;
        touch_button_set_rect(bx & 0xFFFF, ny & 0xFFFF, k ? 0xFD : 0xFF, 0x3C, (volatile unsigned char *)(BTN + 0x818 + 0xA * k));
        if (B8(BTN + 0x818 + 0xA * k + 1)) {
            draw_colour_set(SI(f_3dd54c));
            rect_fill(bx & 0xFFFF, ny & 0xFFFF, k ? 0xFD : 0xFF, 0x3C);
        }
    }
    draw_colour_set(0xAAAAAA);
    for (k = 0; k < 4; k++) {
        e = ENT(k ? 114 + k : 67);
        tw = text_width((const char *)W32(e));
        text_shadow_draw((x + 0xFF * k + ((0xFD - tw) >> 1) + SB8(e + 0x194)) & 0xFFFF, (ny + SB8(e + 0x195) + 0x16) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    }
}
