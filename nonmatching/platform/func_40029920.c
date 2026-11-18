/*
 * ui_clear_data_warning -- stock 0x40029920 (672 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Message box 'Warning !' / 'All saved data will' 'be clear !' (texts 106 and 107..109 of the UI table) in the box at window rectangle 0x42E64520+0x1AA, with the 'Confirm' button (text 95, touch slot 0x42E61FF8+0x200, drawn pressed when its byte +1 is set). Colours from SYS+0x3DD550.
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
#define COL(i) W32(0x40544E68u + 0x3DD550u + 4 * (i))

/* Message box at rectangle roff: title text t centred, three lines b..b+2 at x offset xo, and a
   'Confirm' button (text 95) bd pixels lower than usual, touch slot boff. */
static void msgbox(u32 roff, u32 t, u32 b, u32 xo, u32 bd, u32 boff)
{
    u32 x = WR16(roff), y = WR16(roff + 2), w = WR16(roff + 4), h = WR16(roff + 6);
    u32 iw, x2, y2, bx, bw, by, tw;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    draw_colour_set(COL(0));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    iw = U16(w - 2);
    x2 = x + 2;
    y2 = y + 0x14;
    draw_colour_set(COL(1));
    tw = text_width(TXS(t));
    text_shadow_draw(U16(x2 + ((iw - tw) >> 1) + DX(t)), U16(y2 + DY(t) + 3), TXS(t), 1, 2, 0);
    text_shadow_draw(U16(x2 + DX(b) + xo), U16(y2 + DY(b) + 0x23), TXS(b), 1, 2, 0);
    text_shadow_draw(U16(x2 + DX(b + 1) + xo), U16(y2 + DY(b + 1) + 0x39), TXS(b + 1), 1, 2, 0);
    text_shadow_draw(U16(x2 + DX(b + 2) + xo), U16(y2 + DY(b + 2) + 0x4F), TXS(b + 2), 1, 2, 0);
    bx = x2 + 0x14;
    bw = U16(iw - 0x28);
    by = y2 + 0x6E + bd;
    draw_colour_set(COL(3));
    rect_fill(U16(bx), U16(by), bw, 0x28);
    draw_colour_set(COL(1));
    tw = text_width(TXS(95));
    text_shadow_draw(U16(x2 + ((iw - tw) >> 1) + DX(95)), U16(y2 + DY(95) + 0x7A + bd), TXS(95), 1, 2, 0);
    touch_button_set_rect(U16(bx), U16(y2 + 0x64 + bd), bw, 0x3C, BTN + boff);
    if (BTN[boff + 1]) {
        draw_colour_set(COL(2));
        rect_fill(U16(bx), U16(by), bw, 0x28);
    }
}

void ui_clear_data_warning(void)
{
    msgbox(0x1AA, 106, 107, 0x31, 0, 0x200);
}
