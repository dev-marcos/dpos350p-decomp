/*
 * text_d6f8_shadow_draw -- stock 0x4002D724 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Like text_shadow_draw, for the text routine 0x4002D6F8 (args a, b and the byte e passed on): an n x m block of copies offset by 1..n / 1..m in colour alpha << 24, then the colour (word 0x42E68664) is restored and the text is drawn at (x, y).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void func_4002d82c_flag0(u32 a, u32 b, u32 c, u32 d, u32 e);

void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m,
                           unsigned char alpha)
{
    u32 saved = *(volatile u32 *)0x42E68664u, xx, yy, i, j;
    draw_colour_set((u32)alpha << 24);
    if (n) {
        xx = (unsigned short)x + 1;
        for (i = n; i; i--, xx++) {
            yy = (unsigned short)y + 1;
            for (j = m; j; j--, yy++)
                func_4002d82c_flag0((unsigned short)xx, (unsigned short)yy, a, b, e);
        }
        y = (unsigned short)y;
    }
    draw_colour_set(saved);
    func_4002d82c_flag0((unsigned short)x, y, a, b, e);
}
