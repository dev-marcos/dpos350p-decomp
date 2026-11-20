/*
 * timebase_menu_draw -- stock 0x4001FF74 (5776 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the time-base menu (layout +0xD8..+0xDE): image 0x4008B178 and the label 'Main time base' (text entry 34), or 'Zoom time base' (entry 35) when the split-screen byte SYS+0x2AA is set. The current setting is the byte SYS+0x588 (main) or SYS+0x598 (zoom). Setting 0x2A ('50S') is a 0xA5 x 0x14 button at x + 0x168 in the title row; settings 0x29 .. 0x0C ('20S', '10S', '5S', '2S', '1S', '500mS' .. '5nS' in 1-2-5 steps) are 0x50 x 0x28 buttons in five rows of six, 0x55 apart across and 0x53 down, from (x + 0x14, y + 0x51). The current setting is filled with theme colour +0x10 and has black text, the others theme +0xC with text colour +0x14. Touch records 0x42E61FF8 + 0x426 (setting 0x2A) and + 0x430 + 0xA x (0x29 - setting); a pressed button is shaded with theme +8.
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

static const char tb_text[30][6] = {
    "20S", "10S", "5S", "2S", "1S", "500mS",
    "200mS", "100mS", "50mS", "20mS", "10mS", "5mS",
    "2mS", "1mS", "500uS", "200uS", "100uS", "50uS",
    "20uS", "10uS", "5uS", "2uS", "1uS", "500nS",
    "200nS", "100nS", "50nS", "20nS", "10nS", "5nS",
};

/* fill of a setting button: selected theme +0x10 with black text, else theme +0xC with text colour
   +0x14; returns the text colour */
static u32 fill_colour(u32 sel, u32 v)
{
    if (sel == v) {
        draw_colour_set(W32(TH + 0x10));
        return 0;
    }
    draw_colour_set(W32(TH + 0xC));
    return W32(TH + 0x14);
}

void timebase_menu_draw(void)
{
    u32 x = H16(LAY + 0xD8), y = H16(LAY + 0xDA), w = H16(LAY + 0xDC), h = H16(LAY + 0xDE);
    u32 sel, e, txt, tw, xo, r, rec;
    const char *s;
    int i, j;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x & 0xFFFF, y, w, h);
    sel = B8(SYS + 0x2AA) ? B8(SYS + 0x598) : B8(SYS + 0x588);
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, (y + 0x17) & 0xFFFF, 0x4008B178u, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    e = B8(SYS + 0x2AA) ? ENT(35) : ENT(34);
    text_shadow_draw((x + SB8(e + 0x194) + 0x41) & 0xFFFF, (y + 0x17 + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
    half_42e68be2_set(2);

    xo = x + 0x168;
    r = y + 0x18;
    txt = fill_colour(sel, 0x2A);
    rect_fill(xo & 0xFFFF, r & 0xFFFF, 0xA5, 0x14);
    draw_colour_set(txt);
    s = "50S";
    tw = text_width(s);
    text_draw((((x + 0x14) & 0xFFFF) + 0x154 + ((0xA5 - tw) >> 1)) & 0xFFFF, (y + 0x1B) & 0xFFFF, s);
    touch_button_set_rect(xo & 0xFFFF, (y + 0xC) & 0xFFFF, 0xA5, 0x2C, (volatile unsigned char *)(BTN + 0x426));
    if (B8(BTN + 0x427)) {
        draw_colour_set(W32(TH + 8));
        rect_fill(xo & 0xFFFF, r & 0xFFFF, 0xA5, 0x14);
    }

    r = y + 0x51;
    for (i = 0; i < 5; i++, r += 0x53) {
        for (j = 0; j < 6; j++) {
            int k = 6 * i + j;
            xo = x + 0x14 + 0x55 * j;
            rec = 0x430 + 0xA * k;
            txt = fill_colour(sel, 0x29 - k);
            rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x50, 0x28);
            draw_colour_set(txt);
            tw = text_width(tb_text[k]);
            text_draw((((0x50 - tw) >> 1) + (xo & 0xFFFF)) & 0xFFFF, (r + 0xC) & 0xFFFF, tb_text[k]);
            touch_button_set_rect(xo & 0xFFFF, (r - 0xF) & 0xFFFF, 0x50, 0x46, (volatile unsigned char *)(BTN + rec));
            if (B8(BTN + rec + 1)) {
                draw_colour_set(W32(TH + 8));
                rect_fill(xo & 0xFFFF, r & 0xFFFF, 0x50, 0x28);
            }
        }
    }
}
