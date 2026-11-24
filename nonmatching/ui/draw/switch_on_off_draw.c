/*
 * switch_on_off_draw -- stock 0x40019B8C (240 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws an ON/OFF switch at (x, y): a 60 x 28 box, then the 30 x 28 half (right for on, left for off) and the text "ON" or "OFF" (strings at 0x40019DB0 / 0x40019DB4), with the colours SYS+0x3DD568.. . Font word 0x42E68C74 is set to 0x42E64F58 and half 0x42E68BE2 to 2 for the drawing, and restored after.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "shared/access.h"

void switch_on_off_draw(u32 x, u32 y, u32 on)
{
    volatile u32 *t = (volatile u32 *)(0x40544E68u + 0x3DD568u);
    u32 font = W32(0x42E68C74u);
    unsigned short h = H16(0x42E68BE2u);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E64F58u);
    draw_colour_set(t[0]);
    rect_fill(x, (unsigned short)(y + 2), 0x3C, 0x1C);
    if (on) {
        draw_colour_set(t[1]);
        rect_fill((unsigned short)(x + 0x1E), (unsigned short)(y + 2), 0x1E, 0x1C);
        draw_colour_set(t[3]);
        text_draw((unsigned short)(x + 7), (unsigned short)(y + 10), (const char *)0x40019DB0u);
    } else {
        draw_colour_set(t[2]);
        rect_fill(x, (unsigned short)(y + 2), 0x1E, 0x1C);
        draw_colour_set(t[3]);
        text_draw((unsigned short)(x + 0x23), (unsigned short)(y + 10), (const char *)0x40019DB4u);
    }
    word_42e68c74_set(font);
    half_42e68be2_set((unsigned char)h);
}
