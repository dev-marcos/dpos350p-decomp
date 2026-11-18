/*
 * ui_language_menu_draw -- stock 0x400292E4 (900 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Language menu in the window rectangle 0x42E64520+0x84: four rows 60 pixels apart, each with the image 0x400891D8 and the name of a language (the strings at 0x40090430, 0x40090438, 0x4008E7B4, 0x4008FF6C: Chinese, 'English', Russian, 'Portugues' with accent), touch slots 0x42E61FF8+0x156 + 10 * i. A row is drawn selected when its button byte +1 is set or when the byte SYS+0x7AC (menu language, 1..4) is i + 1. Colours from SYS+0x3DD550.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
u32 text_width(const char *s);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define TXE(k) (0x42C2FE74u + 0x1A0u * (k))
#define TXS(k) ((const char *)W32(TXE(k)))
#define DX(k) (*(volatile signed char *)(TXE(k) + 0x194))
#define DY(k) (*(volatile signed char *)(TXE(k) + 0x195))
#define U16(v) ((unsigned short)(v))

void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
#define COL(i) W32(0x40544E68u + 0x3DD550u + 4 * (i))

void ui_language_menu_draw(void)
{
    static const u32 name[4] = {0x40090430u, 0x40090438u, 0x4008E7B4u, 0x4008FF6Cu};
    u32 x = WR16(0x84), y = WR16(0x86), w = WR16(0x88), h = WR16(0x8A), bx = x + 2, row, i;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(COL(0));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    for (i = 0, row = y + 0x14; i < 4; i++, row += 0x3C) {
        draw_colour_set(COL(1));
        draw_text_shadowed(U16(bx + 0x14), U16(row), 0x400891D8u, 2, 2, 0);
        draw_colour_set(COL(1));
        text_shadow_draw(U16(bx + 0x41), U16(row + 3), (const char *)name[i], 1, 2, 0);
        touch_button_set_rect(U16(bx), U16(row - 0x14), w, 0x3C, BTN + 0x156 + 10 * i);
        if (BTN[0x157 + 10 * i] || *(volatile unsigned char *)(0x40544E68u + 0x7AC) == i + 1) {
            draw_colour_set(COL(2));
            rect_fill(U16(bx), U16(row - 0x14), w, 0x3C);
        }
    }
}
