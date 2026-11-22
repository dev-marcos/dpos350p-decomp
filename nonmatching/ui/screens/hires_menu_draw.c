/*
 * hires_menu_draw -- stock 0x40021630 (2036 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the high-resolution mode menu (layout +0xE6..+0xEC): image 0x4008C3FC, the label of text entry 36 ('High resolution mode') followed by the text 0x4008F8CC, then nine option buttons for the byte SYS+0x660: option 0 is 0x19A wide and shows entry 140 ('OFF') above '8 bit'; options 1 .. 8 ('9 bit' .. '16 bit', strings 0x400903D8 + 8 x i) are 0x64 x 0x2D buttons in two rows of four, 0x69 apart. The selected option is filled with theme colour +0x10 and has black text, the others theme +0xC with text colour +0x14. Touch records 0x42E61FF8 + 0x55C + 0xA x i (0x41 high); a pressed option is shaded with theme +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

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

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);

void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);

/* fill and text colour of option i */
static void option_colours(int i, u32 *txt)
{
    if (B8(SYS + 0x660) == i) {
        draw_colour_set(W32(TH + 0x10));
        *txt = 0;
    } else {
        draw_colour_set(W32(TH + 0xC));
        *txt = W32(TH + 0x14);
    }
}

void hires_menu_draw(void)
{
    u32 x = H16(LAY + 0xE6), y = H16(LAY + 0xE8), w = H16(LAY + 0xEA), h = H16(LAY + 0xEC);
    u32 e = ENT(36), e2 = ENT(140), r, txt, tw, xo, yb;
    const char *s;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x17) & 0xFFFF, 0x4008C3FCu, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (y + 0x14 + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    tw = text_width((const char *)W32(e));
    text_shadow_draw((tw + x + SB8(e + 0x194) + 0x49) & 0xFFFF, (y + 0x14 + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)0x4008F8CCu, 1, 2, 0);

    r = (y + 0x50) & 0xFFFF;
    option_colours(0, &txt);
    rect_fill((x + 0x14) & 0xFFFF, r, 0x19A, 0x2D);
    draw_colour_set(txt);
    tw = text_width((const char *)W32(e2));
    text_draw((x + ((0x19A - tw) >> 1) + SB8(e2 + 0x194) + 0x14) & 0xFFFF, (r + SB8(e2 + 0x195) + 6) & 0xFFFF, (const char *)W32(e2));
    tw = text_width((const char *)0x400903D8u);
    text_draw((x + 0x14 + ((0x19A - tw) >> 1)) & 0xFFFF, (r + 0x19) & 0xFFFF, (const char *)0x400903D8u);
    touch_button_set_rect((x + 0x14) & 0xFFFF, (r - 0xA) & 0xFFFF, 0x19A, 0x41, (volatile unsigned char *)(BTN + 0x55C));
    if (B8(BTN + 0x55D)) {
        draw_colour_set(W32(TH + 8));
        rect_fill((x + 0x14) & 0xFFFF, r, 0x19A, 0x2D);
    }

    for (i = 1; i <= 8; i++) {
        if (i == 1 || i == 5)
            r += 0x5A;
        yb = (r - 0xA) & 0xFFFF;
        xo = x + 0x14 + 0x69 * ((i - 1) & 3);
        s = (const char *)(0x400903D8u + 8 * i);
        option_colours(i, &txt);
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x64, 0x2D);
        draw_colour_set(txt);
        tw = text_width(s);
        text_draw((xo + ((0x64 - tw) >> 1)) & 0xFFFF, (r + 0xF) & 0xFFFF, s);
        touch_button_set_rect(xo & 0xFFFF, yb, 0x64, 0x41, (volatile unsigned char *)(BTN + 0x55C + 0xA * i));
        if (B8(BTN + 0x55D + 0xA * i)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x64, 0x2D);
        }
    }
}
