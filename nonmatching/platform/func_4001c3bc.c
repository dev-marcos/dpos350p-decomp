/*
 * system_menu_draw -- stock 0x4001C3BC (2628 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the system settings menu (layout +0x5A..+0x60). Rows with an image, a text-table label and the arrow icon at x + 0x145: 'Configuration settings' (entry 9), 'System calibration' (10), 'USB connect' (11), then 'Automatic shutdown' (12) with five 0x41 x 0x1E options 'OFF', '15 min', '30 min', '45 min', '1 hour' for the word SYS+0x658 (0, 15, 30, 45, 60 minutes), then 'Language settings' (13), 'Factory settings' (14) and 'Disk formatting' (15). A pressed row or option stores its number in layout byte +0x59 (rows 1, 2, 3, 5, 5, 6; options 0 .. 4) and is shaded with theme colour +8. Touch records 0x42E61FF8 + 0xFC, +0x17E, +0x192, +0x1A6 .. +0x1CE, +0x1D8, +0x1E2, +0x1F6.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed, ... */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ENT(n) (TXT + 0x1A0 * (n))

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records; byte 0 is the pending-touch flag */
#define B8(a) (*(volatile unsigned char *)(a))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
void text_draw(u32 x, u32 y, const char *s);
u32 text_width(const char *s);
void icon_19c7c_draw(u32 x, u32 y);

static void label_row(u32 x, u32 y, u32 img, u32 e)
{
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, img, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + 0x41 + SB8(e + 0x194)) & 0xFFFF, (y + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
}

/* row with the arrow icon; the pressed colour is set even when the row is not pressed */
static void arrow_row(u32 x, u32 y, u32 w, u32 img, u32 e, u32 rec, u32 n)
{
    label_row(x, y, img, e);
    icon_19c7c_draw((x + 0x145) & 0xFFFF, y & 0xFFFF);
    touch_button_set_rect(x, (y - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + rec));
    draw_colour_set(W32(TH + 8));
    if (B8(BTN + rec + 1)) {
        B8(LAY + 0x59) = n;
        rect_fill(x, (y - 0x14) & 0xFFFF, w, 0x3C);
    }
}

void system_menu_draw(void)
{
    u32 x = H16(LAY + 0x5A), y = H16(LAY + 0x5C), w = H16(LAY + 0x5E), h = H16(LAY + 0x60);
    u32 r, xo, tw, rec;
    const char *s;
    int k;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    r = y + 0x14;
    arrow_row(x, r, w, 0x4008B8A8u, ENT(9), 0xFC, 1);
    r += 0x3C;
    arrow_row(x, r, w, 0x40089BE8u, ENT(10), 0x17E, 2);
    r += 0x3C;
    arrow_row(x, r, w, 0x40088EF8u, ENT(11), 0x192, 3);
    r += 0x3C;
    draw_colour_set(W32(TH));
    rect_fill((x + 0xA) & 0xFFFF, (r - 0x10) & 0xFFFF, (w - 0x1E) & 0xFFFF, 0x78);
    label_row(x, r, 0x40089068u, ENT(12));
    r += 0x3C;
    for (k = 0; k < 5; k++) {
        xo = x + 0x14 + 0x45 * k;
        rec = 0x1A6 + 0xA * k;
        draw_colour_set(W32(SYS + 0x658) == (u32)(15 * k) ? W32(TH + 0x10) : W32(TH + 0xC));
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x41, 0x1E);
        draw_colour_set(W32(SYS + 0x658) != (u32)(15 * k) ? W32(TH + 0x14) : 0);
        s = k == 0 ? "OFF" : (const char *)(0x400902F8u + 8 * (k - 1));
        tw = text_width(s);
        text_draw((xo + ((0x41 - tw) >> 1)) & 0xFFFF, (r + 8) & 0xFFFF, s);
        touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, 0x41, 0x3C, (volatile unsigned char *)(BTN + rec));
        if (B8(BTN + rec + 1)) {
            B8(LAY + 0x59) = k;
            draw_colour_set(W32(TH + 8));
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x41, 0x1E);
        }
    }
    word_42e68c74_set(0x42E658D8u);
    r += 0x48;
    arrow_row(x, r, w, 0x400891D8u, ENT(13), 0x1D8, 5);
    word_42e68c74_set(0x42E658D8u);
    r += 0x3C;
    arrow_row(x, r, w, 0x40089348u, ENT(14), 0x1E2, 5);
    r += 0x3C;
    arrow_row(x, r, w, 0x400894B8u, ENT(15), 0x1F6, 6);
}
