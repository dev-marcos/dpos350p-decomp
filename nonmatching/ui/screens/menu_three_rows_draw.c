/*
 * menu_three_rows_draw -- stock 0x4001E8E0 (764 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the three-row menu box of layout 0x42E64520 +0xAE..+0xB4: fills it with theme colour SYS+0x3DD550, then for each row (0x3C high) the image 0x4008A038 / 0x4008A1A8 / 0x4008A318, the label of text-table entry 110 + row (with its per-language x / y offsets), the icon 0x40019C7C at the right, and a touch button (records 0x42E61FF8 +0x340 / +0x34A / +0x354) that is shaded with theme colour +8 while pressed.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);

void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
void icon_19c7c_draw(u32 x, u32 y);

#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */

void menu_three_rows_draw(void)
{
    u32 x = H16(0x42E64520u + 0xAE), y = H16(0x42E64520u + 0xB0);
    u32 w = H16(0x42E64520u + 0xB2), h = H16(0x42E64520u + 0xB4);
    u32 sl, e;
    volatile unsigned char *rec;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    for (i = 0; i < 3; i++) {
        sl = y + 0x14 + 0x3C * i;
        e = TXT + 0x1A0 * (110 + i);
        rec = (volatile unsigned char *)(0x42E61FF8u + 0x340 + 0xA * i);
        draw_colour_set(W32(TH + 4));
        draw_text_shadowed((x + 0x14) & 0xFFFF, sl & 0xFFFF, 0x4008A038u + 0x170 * i, 2, 2, 0);
        draw_colour_set(W32(TH + 4));
        text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (sl + 3 + SB8(e + 0x195)) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        icon_19c7c_draw((x + 0x14A) & 0xFFFF, sl & 0xFFFF);
        touch_button_set_rect(x, (sl - 0x14) & 0xFFFF, w, 0x3C, rec);
        if (rec[1]) {
            draw_colour_set(W32(TH + 8));
            rect_fill(x, (sl - 0x14) & 0xFFFF, w, 0x3C);
        }
    }
}
