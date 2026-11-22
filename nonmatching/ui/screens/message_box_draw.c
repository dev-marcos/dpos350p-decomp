/*
 * message_box_draw -- stock 0x40034134 (336 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws a message box with the text s near (x, y): width = text width (0x4002D164) + 50, height 50; align 0 centres it above the point (y - 25), 2 puts it to the upper left (x - w, y - 50), both clamped at 0; 1 and others leave (x, y). Saves the area to 0x420E4E84 first, stores the box in 0x42E68C38, draws a black box with a 0x282828 inside and the text (colour word 0x409223BC, shadow 1 x 2), then presents the rectangle.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void word_42e68c74_set(u32 a);
void half_42e68be2_set(u32 a);
int text_width(const char *s);
void fb_rect_copy_to_e4e84(u32 x0, u32 y0, u32 x1, u32 y1);
void fb_target_back(void);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1);

void message_box_draw(int x, int y, u32 text, int align)
{
    volatile unsigned short *b = (volatile unsigned short *)0x42E68C38u;
    const char *s = (const char *)text;
    int w;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    w = (unsigned short)(text_width(s) + 0x32);
    if (align == 0) {
        y -= 0x19;
        x -= w / 2;
    } else if (align == 2) {
        x -= w;
        y -= 0x32;
    }
    if (align == 0 || align == 2) {
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
    }
    b[3] = 0x32;
    b[0] = x;
    b[1] = y;
    b[2] = w;
    fb_rect_copy_to_e4e84((unsigned short)x, (unsigned short)y, (unsigned short)(w + x + 1), (unsigned short)(y + 0x33));
    fb_target_back();
    draw_colour_set(0);
    rect_fill((unsigned short)x, (unsigned short)y, w, 0x32);
    draw_colour_set(0x282828);
    rect_fill((unsigned short)(x + 1), (unsigned short)(y + 1), (unsigned short)(w - 2), 0x30);
    draw_colour_set(*(volatile u32 *)0x409223BCu);
    text_shadow_draw((unsigned short)(x + 0x19), (unsigned short)(y + 0x11), s, 1, 2, 0);
    fb_present_rect((unsigned short)x, (unsigned short)y, (unsigned short)(w + x), (unsigned short)(y + 0x32));
}
