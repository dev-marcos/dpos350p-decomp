/*
 * battery_icon_draw -- stock 0x4001C200 (324 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the battery icon in the rectangle at 0x42E6455E (x, y, w, h): black background, a 13 x 24 outline and the 5 x 2 terminal, a fill of 2 * level (SYS+0x778) pixels, and when SYS+0x779 is set a white charging mark (a bar and a 4-row triangle).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void rect_fill(u32 a, u32 b, u32 c, u32 d);
void func_4002ecc4_span(u32 a, u32 b, u32 c);
void text_draw(u32 x, u32 y, const char *s);
void pixel_put(u32 x, u32 y);
#define W32(a) (*(volatile u32 *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void rect_outline_draw(u32 a, u32 b, u32 c, u32 d);

void battery_icon_draw(void)
{
    u32 x = H16(0x42E64520u + 0x3E), y = H16(0x42E64520u + 0x40);
    u32 w = H16(0x42E64520u + 0x42), h = H16(0x42E64520u + 0x44), lv;
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    draw_colour_set(0xDDDDDD);
    rect_outline_draw(x, (unsigned short)(y + 2), 13, 24);
    rect_fill((unsigned short)(x + 4), y, 5, 2);
    draw_colour_set(0x88FF);
    lv = SYS8(0x778);
    rect_fill((unsigned short)(x + 2), (unsigned short)(y + 24 - 2 * lv), 9, 2 * lv);
    if (SYS8(0x779) == 0)
        return;
    draw_colour_set(0xFFFFFF);
    rect_fill((unsigned short)(x + 5), (unsigned short)(y + 8), 3, 10);
    func_4002ecc4_span((unsigned short)(y + 0x11), (unsigned short)(x + 3), (unsigned short)(x + 9));
    func_4002ecc4_span((unsigned short)(y + 0x12), (unsigned short)(x + 4), (unsigned short)(x + 8));
    func_4002ecc4_span((unsigned short)(y + 0x13), (unsigned short)(x + 5), (unsigned short)(x + 7));
    func_4002ecc4_span((unsigned short)(y + 0x14), (unsigned short)(x + 6), (unsigned short)(x + 6));
}
