/*
 * trigger_menu_draw -- stock 0x40021E68 (2724 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the trigger menu (layout +0xF4..+0xFA) in four sections, each an image, a text-table label and a row of 0x1E high option buttons: 'Trigger mode' (text entry 37) with 'Auto', 'Single', 'Normal' (entries 41 .. 43, 0x86 wide) for the byte SYS+0x27A; 'Trigger edge' (38) with 'Rising', 'Falling' (44, 45, 0xCB wide) for SYS+0x27B; 'Trigger channel' (39) with 'CH1', 'CH2' for SYS+0x27C; 'HF suppresstion' (40) with 'OFF' (entry 140), 'L1', 'L2', 'L3' for SYS+0x284. The selected option is filled with theme colour +0x10 and black text, the others theme +0xC with text colour +0x14; a pressed option is shaded with theme +8. Touch records 0x42E61FF8 + 0x5B6, +0x5C0, +0x5CA, +0x5D4, +0x5DE, +0x5E8, +0x5F2, +0x5FC, +0x606, +0x610, +0x61A.
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

static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
}

/* one option button: selected when the setting byte at sel equals v; the text is centred, then moved
   by dx / dy (the text-table offsets, 0 for the in-code texts) */
static void option(u32 xo, u32 r, u32 wd, u32 sel, u32 v, const char *s, int dx, int dy, u32 rec)
{
    u32 txt, tw;

    if (B8(sel) == v) {
        draw_colour_set(W32(TH + 0x10));
        txt = 0;
    } else {
        draw_colour_set(W32(TH + 0xC));
        txt = W32(TH + 0x14);
    }
    rect_fill(xo & 0xFFFF, r & 0xFFFF, wd, 0x1E);
    draw_colour_set(txt);
    tw = text_width(s);
    text_draw((xo + ((wd - tw) >> 1) + dx) & 0xFFFF, (r + dy + 8) & 0xFFFF, s);
    touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, wd, 0x3C, (volatile unsigned char *)(BTN + rec));
    if (B8(BTN + rec + 1)) {
        draw_colour_set(W32(TH + 8));
        rect_fill(xo & 0xFFFF, r & 0xFFFF, wd, 0x1E);
    }
}

static void ent_option(u32 xo, u32 r, u32 wd, u32 sel, u32 v, u32 e, u32 rec)
{
    option(xo, r, wd, sel, v, (const char *)W32(e), SB8(e + 0x194), SB8(e + 0x195), rec);
}

void trigger_menu_draw(void)
{
    u32 x = H16(LAY + 0xF4), y = H16(LAY + 0xF6), w = H16(LAY + 0xF8), h = H16(LAY + 0xFA);
    u32 r;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);

    r = y + 0x19;
    label_row(x, r, 0x4008ABB8u, ENT(37));
    r += 0x37;
    ent_option(x + 0x14, r, 0x86, SYS + 0x27A, 0, ENT(41), 0x5B6);
    ent_option(x + 0x9E, r, 0x86, SYS + 0x27A, 1, ENT(42), 0x5C0);
    ent_option(x + 0x128, r, 0x86, SYS + 0x27A, 2, ENT(43), 0x5CA);

    r += 0x41;
    label_row(x, r, 0x4008AD28u, ENT(38));
    r += 0x37;
    ent_option(x + 0x14, r, 0xCB, SYS + 0x27B, 0, ENT(44), 0x5D4);
    ent_option(x + 0xE3, r, 0xCB, SYS + 0x27B, 1, ENT(45), 0x5DE);

    r += 0x41;
    label_row(x, r, 0x4008AE98u, ENT(39));
    r += 0x37;
    option(x + 0x14, r, 0xCB, SYS + 0x27C, 0, "CH1", 0, 0, 0x5E8);
    option(x + 0xE3, r, 0xCB, SYS + 0x27C, 1, "CH2", 0, 0, 0x5F2);

    r += 0x41;
    label_row(x, r, 0x4008B008u, ENT(40));
    r += 0x37;
    ent_option(x + 0x14, r, 0x63, SYS + 0x284, 0, ENT(140), 0x5FC);
    option(x + 0x7B, r, 0x64, SYS + 0x284, 1, "L1", 0, 0, 0x606);
    option(x + 0xE3, r, 0x64, SYS + 0x284, 2, "L2", 0, 0, 0x610);
    option(x + 0x14B, r, 0x63, SYS + 0x284, 3, "L3", 0, 0, 0x61A);
}
