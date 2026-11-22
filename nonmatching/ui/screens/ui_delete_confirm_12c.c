/*
 * ui_delete_confirm_12c -- stock 0x40024E38 (656 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Dialog 'Delete selected <n> items ?' (n = u16 SYS+0x88BE) in the box at window rectangle 0x42E64520+0x12C, with the 'Confirm' button (touch slot 0x42E61FF8+0x7FA, drawn pressed when its byte +1 is set). Colours from SYS+0x3DD54C. Unlike the other two copies, the count is placed after the x offset of the first text as well.
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
void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m,
                           unsigned char alpha);
#define COL(i) W32(0x40544E68u + 0x3DD54Cu + 4 * (i))

/* "Delete selected" <count SYS+0x88BE> "items" " ?" over a box at rectangle roff of the window
   table 0x42E64520, then a 240 x 50 button labelled "Confirm" (text 95) with touch slot boff. */
static void delete_dialog(u32 roff, u32 boff, int dx_in_x2)
{
    u32 x = WR16(roff), y = WR16(roff + 2), w = WR16(roff + 4), h = WR16(roff + 6);
    u32 cx = x + (w >> 1), ty = y + 0x14, x1, x2, x3, bx, by, tw;
    draw_colour_set(COL(1));
    rect_fill(x, y, w, h);
    draw_colour_set(COL(2));
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E658D8u);
    x1 = U16(x + 0x38);
    text_shadow_draw(U16(x1 + DX(118)), U16(ty + DY(118)), TXS(118), 1, 2, 0);
    x2 = U16(H16(0x42E68BE4u) + x1 + (dx_in_x2 ? DX(118) : 0) + 8);
    text_d6f8_shadow_draw(x2, U16(ty), H16(0x40544E68u + 0x88BE), 0, 0, 1, 2, 0);
    x3 = U16(H16(0x42E68BE4u) + x2 + 8);
    text_shadow_draw(U16(x3 + DX(120)), U16(ty + DY(120)), TXS(120), 1, 2, 0);
    text_shadow_draw(U16(H16(0x42E68BE4u) + x3), U16(ty), " ?", 1, 2, 0);
    bx = cx - 0x78;
    by = y + 0x50;
    draw_colour_set(COL(4));
    rect_fill(U16(bx), U16(by), 0xF0, 0x32);
    touch_button_set_rect(U16(bx), U16(by - 10), 0xF0, 0x46, BTN + boff);
    if (BTN[boff + 1]) {
        draw_colour_set(COL(0));
        rect_fill(U16(bx), U16(by), 0xF0, 0x32);
    }
    draw_colour_set(COL(2));
    tw = text_width(TXS(95));
    text_shadow_draw(U16(x + ((w - tw) >> 1) + DX(95)), U16(by + DY(95) + 0x14), TXS(95), 1, 2, 0);
}

void ui_delete_confirm_12c(void)
{
    delete_dialog(0x12C, 0x7FA, 1);
}
