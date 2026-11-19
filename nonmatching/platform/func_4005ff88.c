/*
 * numpad_draw -- stock 0x4005FF88 (1376 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the numeric keypad in the box at 0x42E646F4 (x, y, w, h u16): a dark frame with the title string 0x42E68A8B, then 4 rows x 4 keys of 0x5A x 0x3C, 0x5D x 0x3F apart: digits 7 8 9 / 4 5 6 / 1 2 3 / . 0 and the function keys (colour 0x202020). Each key is a touch button (records 0x42E62A18..0x42E62AAE), highlighted in the colour W32 0x409223B4 when pressed; the keypad mode byte 0x42E68A89 = 1..6 disables some keys (11, 12, 15) and picks the glyphs of keys 7, 11 and 15 and the '+-' of key 12.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define D64(a) (*(volatile double *)(a))
#define BLK 0x42E68668u
#define ADV 0x42E68BE4u

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void text_draw(u32 x, u32 y, const char *s);

#define REC 0x42E61FF8u

void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void touch_button_disable(volatile unsigned char *b);
void glyph_alpha_draw(u32 x, u32 y, u32 s);
u32 func_4002d2d4_d0(u32 lo, u32 hi, u32 dec);
void func_4002d82c_flag0(u32 x, u32 y, u32 lo, u32 hi, u32 dec);
u32 text_width(const char *s);

void numpad_draw(void)
{
    static const unsigned char lab[16] = {7, 8, 9, 100, 4, 5, 6, 100, 1, 2, 3, 100, 100, 0, 100, 100};
    static const unsigned short off[16] = {0xA66, 0xA70, 0xA7A, 0xA98, 0xA48, 0xA52, 0xA5C, 0xAA2,
                                           0xA2A, 0xA34, 0xA3E, 0xAAC, 0xA84, 0xA20, 0xA8E, 0xAB6};
    u32 x0, y0, w, h, x, ry, row, col, k, mode, lb, tw, gx, gs, dis;
    volatile unsigned char *rec;
    const char *s;
    x0 = H16(0x42E646F4u);
    y0 = H16(0x42E646F6u);
    w = H16(0x42E646F8u);
    h = H16(0x42E646FAu);
    draw_colour_set(0);
    rect_fill(x0, y0, w, h);
    y0 += 1;
    x = x0 + 1;
    word_42e68c74_set(0x42E66BD8u);
    half_42e68be2_set(5);
    draw_colour_set(0x202020);
    rect_fill(x & 0xFFFF, y0 & 0xFFFF, (w - 2) & 0xFFFF, 0x3C);
    draw_colour_set(0xCCCCCC);
    text_shadow_draw((x + 0x1E) & 0xFFFF, (y0 + 0x13) & 0xFFFF, (const char *)0x42E68A8Bu, 1, 2, 0);
    ry = y0 + 0x3F;
    k = 0;
    for (row = 4; row; row--) {
        for (col = 0; col < 4; col++, k++, x += 0x5D) {
            lb = lab[k];
            rec = (volatile unsigned char *)(REC + off[k]);
            draw_colour_set(lb == 100 ? 0x202020 : 0x303030);
            rect_fill(x & 0xFFFF, ry & 0xFFFF, 0x5A, 0x3C);
            mode = B8(0x42E68A89u);
            if (mode == 2 || mode == 3)
                dis = k == 11 || k == 15;
            else if (mode == 1)
                dis = k == 12;
            else if (mode == 4)
                dis = k == 15;
            else if (mode == 5)
                dis = k == 11 || k == 12 || k == 15;
            else
                dis = 0;
            if (dis) {
                touch_button_disable(rec);
            } else {
                touch_button_set_rect(x & 0xFFFF, ry & 0xFFFF, 0x5A, 0x3C, rec);
                if (rec[1]) {
                    draw_colour_set(W32(0x409223B4u));
                    rect_fill(x & 0xFFFF, ry & 0xFFFF, 0x5A, 0x3C);
                }
            }
            draw_colour_set(0xCCCCCC);
            if (lb <= 9) {
                tw = func_4002d2d4_d0(lb, 0, 0);
                func_4002d82c_flag0((x + ((0x5A - tw) >> 1)) & 0xFFFF, (ry + 0x12) & 0xFFFF, lb, 0, 0);
                continue;
            }
            gs = 0;
            gx = 0;
            s = 0;
            if (k == 3) {
                glyph_alpha_draw((x + 0x20) & 0xFFFF, (ry + 0x15) & 0xFFFF, 0x40087B94u);
                continue;
            } else if (k == 14) {
                s = (const char *)0x400604E8u;
            } else if (k == 12) {
                mode = B8(0x42E68A89u);
                if (mode == 2 || mode == 3 || mode == 4)
                    s = (const char *)0x400604ECu;
            } else if (k == 7) {
                switch (B8(0x42E68A89u)) {
                case 1: gx = 0x21; gs = 0x40087F10u; break;
                case 2: gx = 0x21; gs = 0x40088094u; break;
                case 3: gx = 0x10; gs = 0x40084E30u; break;
                case 4: gx = 0x1D; gs = 0x400879A0u; break;
                case 5: gx = 0x1F; gs = 0x40087D5Cu; break;
                case 6: gx = 0x23; gs = 0x4008C2B8u; break;
                }
            } else if (k == 11) {
                mode = B8(0x42E68A89u);
                if (mode == 1) {
                    gx = 0x1A; gs = 0x40087548u;
                } else if (mode == 4) {
                    gx = 0x26; gs = 0x4008C60Cu;
                }
            } else if (k == 15) {
                if (B8(0x42E68A89u) == 1) {
                    gx = 0x18; gs = 0x400872B4u;
                }
            }
            if (s) {
                tw = text_width(s);
                text_draw((x + ((0x5A - tw) >> 1)) & 0xFFFF, (ry + 0x12) & 0xFFFF, s);
            } else if (gs) {
                glyph_alpha_draw((x + gx) & 0xFFFF, (ry + 0x16) & 0xFFFF, gs);
            }
        }
        ry += 0x3F;
        x = x0 + 1;
    }
    (void)h;
}
