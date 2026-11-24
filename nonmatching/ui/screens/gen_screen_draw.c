/*
 * gen_screen_draw -- stock 0x40022958 (5000 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window 18 (layout 0x42E64520 + 0x102), the signal generator. Left: four 390 x 60 tabs (touch buttons 0x42E61FF8 + 0x624 + 10 i, labels entries 46..49), the selected one (G+1, G = 0x42E3F984) in 0x303030 with a triangle marker; the read-outs in font 0x42E66BD8: frequency GN32(0x2EFC) as 8 digits + Hz, amplitude GN32(0x2F00) as d.ddd VPP, offset GN32(0x2F04) as [+-]d.ddd VDC, duty GN32(0x2F08) as dd.d %; a 15 x 3 cursor under digit *GN32(4) of the selected row. Right: the arrow pad (four 20-line triangles, grey when their button +0x656 / +0x660 / +0x66A / +0x674 is pressed) and a 4 x 4 menu icon (+0x67E). Bottom: the waveform preview box (button +0x64C) with the white polyline of the GENS(f_fae) points GN16(0x10 + 2 i). When G+0xC is set, the 3 x 5 picker of the 15 waveforms (buttons +0x688 + 10 i, labels entries 50..64, the current one G+0x2EFA highlighted) covers the top. Last, a status bar with the current waveform name. Colours come from the palette 0x409223B4 and the theme SYS+0x3DD54C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "state/gen_block.h"
#include "state/ui_windows.h"
#include "state/ui_texts.h"

#define U16(v)    ((v) & 0xFFFFu)
#define PAL(i)    (*(volatile u32 *)(0x409223B4u + 4 * (i)))
#define THEME(o)  (*(volatile u32 *)(0x40544E68u + 0x3DD54Cu + (o)))
#define GN16(o)    (*(volatile unsigned short *)(0x42E3F984u + (o)))
#define GN32(o)    (*(volatile int *)(0x42E3F984u + (o)))
#define TREC(o)   ((volatile unsigned char *)(0x42E61FF8u + (o)))
#define TXE(k)    (0x42C2FE74u + (k) * 0x1A0u)
#define TXP(k)    (*(const char *volatile *)TXE(k))

static void gs_text(u32 x, u32 y, const char *s)
{
    text_shadow_draw(U16(x), U16(y), s, 1, 2, 0);
}

static void gs_digit(u32 x, u32 y, unsigned short d)
{
    text_d6f8_shadow_draw(U16(x), U16(y), d, 0, 0, 1, 2, 0);
}

static u32 gs_arrow_colour(u32 rec)
{
    return TREC(rec)[1] ? 0x666666 : PAL(5);
}

void gen_screen_draw(void)
{
    u32 x = WREC(18, x), y = WREC(18, y), w = WREC(18, w), h = WREC(18, h);
    u32 i, j, k, cx, cy, sel, cur, x0, y0;
    unsigned short d[8];
    int v;

    byte_42e68be0_set(0);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(PAL(1));
    draw_colour_set(0);
    rect_fill(x, y, w, U16(h));
    draw_colour_set(PAL(1));
    for (i = 0, cy = y; i < 4; i++, cy += 0x3F) {             /* parameter tabs */
        rect_fill(x, U16(cy), 0x186, 0x3C);
        touch_button_set_rect(x, U16(cy), 0x186, 0x3C, TREC(0x624 + 10 * i));
        if (TREC(0x624 + 10 * i)[1]) {
            draw_colour_set(PAL(3));
            rect_fill(x, U16(cy), 0x186, 0x3C);
            draw_colour_set(PAL(1));
        }
    }
    cy = GENS(f_001) * 63 + y;                                       /* selected tab */
    draw_colour_set(0x303030);
    rect_fill(x, U16(cy), 0x186, 0x3C);
    draw_colour_set(PAL(5));
    for (i = 0; i < 0x23; i++)
        hspan_fill(U16(cy + 0x22 - i), x, U16(x + i));
    word_42e68c74_set(0x42E64A98u);
    draw_colour_set(0xC0C0C0);
    for (i = 0; i < 4; i++)
        gs_text(x + 0x23 + TEXT_DX(UI_TEXT(46 + i)), y + 0x17 + 0x3F * i + TEXT_DY(UI_TEXT(46 + i)), TXP(46 + i));

    half_42e68be2_set(2);                                      /* units */
    word_42e68c74_set(0x42E658D8u);
    gs_text(x + 0x9E + 0xB9, y + 0x1E, "Hz");
    gs_text(x + 0x9E + 0x66, y + 0x5D, "VPP");
    gs_text(GN32(0x2F04) == 0 ? x + 0x9E + 0x66 : x + 0x9E + 0x7A, y + 0x9C, "VDC");
    gs_text(x + 0x9E + 0x53, y + 0xDB, "%");

    byte_42e68be0_set(1);                                      /* read-outs */
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E66BD8u);
    draw_colour_set(0xFFFFFF);
    v = GN32(0x2EFC);
    d[0] = v / 10000000;
    d[1] = v % 10000000 / 1000000;
    d[2] = v % 1000000 / 100000;
    d[3] = v % 100000 / 10000;
    d[4] = v % 10000 / 1000;
    d[5] = v % 1000 / 100;
    d[6] = v % 100 / 10;
    d[7] = v % 10;
    for (i = 0, cx = x + 0x9E; i < 8; i++) {
        gs_digit(cx, y + 0x13, d[i]);
        cx += (i == 1 || i == 4) ? 0x1E : 0x14;
    }
    v = GN32(0x2F00);
    d[0] = v % 10000 / 1000;
    d[1] = v % 1000 / 100;
    d[2] = v % 100 / 10;
    d[3] = v % 10;
    cx = x + 0x9E;
    gs_digit(cx, y + 0x52, d[0]);
    cx += 0x14;
    gs_text(cx, y + 0x52, ".");
    for (i = 1, cx += 0x14; i < 4; i++, cx += 0x14)
        gs_digit(cx, y + 0x52, d[i]);
    {
        u32 o = (u32)GN32(0x2F04);
        if ((int)o < 0)
            o = -o;
        d[0] = o % 10000 / 1000;
        d[1] = o % 1000 / 100;
        d[2] = o % 100 / 10;
        d[3] = o % 10;
    }
    for (i = 0, cx = x + 0x9E; i < 4; i++, cx += 0x14) {
        if (i == 0) {
            if (GN32(0x2F04) > 0) {
                gs_text(cx, y + 0x91, "+");
                cx += 0x14;
            } else if (GN32(0x2F04) < 0) {
                gs_text(cx, y + 0x91, "-");
                cx += 0x14;
            }
        }
        gs_digit(cx, y + 0x91, d[i]);
        if (i == 0) {
            cx += 0x14;
            gs_text(cx, y + 0x91, ".");
        }
    }
    v = GN32(0x2F08);
    d[0] = v % 1000 / 100;
    d[1] = v % 100 / 10;
    d[2] = v % 10;
    for (i = 0, cx = x + 0x9E; i <= 2; i++, cx += 0x14) {
        gs_digit(cx, y + 0xD0, d[i]);
        if (i == 1) {
            cx += 0x14;
            gs_text(cx, y + 0xD0, ".");
        }
    }

    byte_42e68be0_set(0);                                      /* edit cursor */
    draw_colour_set(PAL(5));
    sel = GENS(f_001);
    cur = **(volatile unsigned char *volatile *)(0x42E3F984u + 4);
    cx = cur * 20 + U16(x + 0x9E - 7);
    if (sel == 0) {
        if (cur >= 2)
            cx += 10;
        if (cur > 4)
            cx += 10;
    } else if (sel == 1) {
        if (cur != 0)
            cx += 20;
    } else if (sel == 2) {
        if (cur != 0)
            cx += 20;
        if (GN32(0x2F04) != 0)
            cx += 20;
    } else if (sel == 3) {
        if (cur >= 2)
            cx += 20;
    }
    rect_fill(U16(cx), U16(sel * 63 + y + 0x13 + 0x1E), 0xF, 3);

    draw_colour_set(PAL(1));                                   /* arrow pad */
    rect_fill(U16(x + 0x189), y, 0xF9, 0xF9);
    draw_colour_set(PAL(5));
    x0 = U16(x + 0x206);
    y0 = U16(y + 0x7D);
    draw_colour_set(gs_arrow_colour(0x656));
    for (i = 0; i < 20; i++)
        hspan_fill(U16(y0 - 0x54 + i), U16(x0 - i), U16(x0 + i));
    draw_colour_set(gs_arrow_colour(0x660));
    for (i = 0; i < 20; i++)
        hspan_fill(U16(y0 + 0x54 - i), U16(x0 - i), U16(x0 + i));
    draw_colour_set(gs_arrow_colour(0x66A));
    for (i = 0; i < 20; i++) {
        hspan_fill(U16(y0 - i), U16(x0 - 0x54 + i), U16(x0 - 0x41));
        hspan_fill(U16(y0 + i), U16(x0 - 0x54 + i), U16(x0 - 0x41));
    }
    draw_colour_set(gs_arrow_colour(0x674));
    for (i = 0; i < 20; i++) {
        hspan_fill(U16(y0 - i), U16(x0 + 0x41), U16(x0 + 0x54 - i));
        hspan_fill(U16(y0 + i), U16(x0 + 0x41), U16(x0 + 0x54 - i));
    }
    draw_colour_set(TREC(0x67E)[1] ? 0x666666 : 0x333333);
    for (i = 0, cy = y + 0xF; i < 4; i++, cy += 0xB)
        for (j = 0, cx = x + 0x249; j < 4; j++, cx += 0xB)
            rect_fill(U16(cx), U16(cy), 9, 9);
    touch_button_set_rect(U16(x + 0x1E3), U16(y), 0x46, 0x5A, TREC(0x656));
    touch_button_set_rect(U16(x + 0x1E3), U16(y + 0x9F), 0x46, 0x5A, TREC(0x660));
    touch_button_set_rect(U16(x + 0x189), U16(y + 0x5A), 0x5A, 0x46, TREC(0x66A));
    touch_button_set_rect(U16(x + 0x229), U16(y + 0x5A), 0x5C, 0x46, TREC(0x674));
    touch_button_set_rect(U16(x + 0x229), U16(y), 0x5C, 0x5A, TREC(0x67E));

    word_42e68c74_set(0x42E658D8u);                            /* waveform preview */
    half_42e68be2_set(1);
    draw_colour_set(PAL(1));
    rect_fill(x, U16(y + 0xFC), U16(w - 3), 0xBE);
    touch_button_set_rect(x, U16(y + 0xFC), U16(w - 3), 0xBE, TREC(0x64C));
    draw_colour_set(0xFFFFFF);
    if ((u32)(GENS(f_fae) - 1) > 1)
        for (i = 1; i < (u32)(GENS(f_fae) - 1); i++) {
            u32 a = GN16(0x10 + 2 * (i - 1)), b = GN16(0x12 + 2 * (i - 1));
            u32 t = U16(y + 0xFC);
            if (a < b)
                vline_draw(U16(x + i + 0x2B), U16(a + t + 0x1E), U16(b + t + 0x1E));
            else
                vline_draw(U16(x + i + 0x2B), U16(b + t + 0x1E), U16(a + t + 0x1E));
        }

    if (GENS(f_00c)) {                                             /* waveform picker */
        word_42e68c74_set(0x42E64A98u);
        draw_colour_set(PAL(1));
        rect_fill(x, y, U16(w - 3), 0xF9);
        draw_colour_set(PAL(4));
        k = 0;
        for (i = 0, cy = U16(y + 0x16); i < 3; i++, cy += 0x50)
            for (j = 0, cx = U16(x + 0x16); j < 5; j++, cx += 0x79, k++) {
                volatile unsigned char *b = TREC(k < 14 ? 0x688 + 10 * k : 0x714);
                draw_colour_set(k < 14 ? THEME(0x10) : 0x444444);
                rect_fill(U16(cx), U16(cy), 0x75, 0x2D);
                touch_button_set_rect(U16(cx - 2), U16(cy - 0x11), 0x79, 0x4F, b);
                if (b[1]) {
                    draw_colour_set(THEME(0));
                    rect_fill(U16(cx), U16(cy), 0x75, 0x2D);
                    draw_colour_set(THEME(0x10));
                }
            }
        draw_colour_set(0xAAAAAA);
        k = 0;
        for (i = 0, cy = U16(y + 0x16); i < 3; i++, cy += 0x50)
            for (j = 0, cx = U16(x + 0x16); j < 5; j++, cx += 0x79, k++) {
                u32 e = U16(k + 0x32);
                int dw = (int)(0x75 - U16(text_width(TXP(e)))) / 2;
                gs_text(cx + dw + TEXT_DX(UI_TEXT(e)), cy + TEXT_DY(UI_TEXT(e)) + 0xF, TXP(e));
            }
        {
            u32 wv = GENS(waveform), e;
            int dw;
            cx = 0x79 * (wv % 5) + U16(x + 0x16);
            cy = U16(y + 0x16) + 0x50 * (wv / 5);
            draw_colour_set(THEME(0x14));
            rect_fill(U16(cx), U16(cy), 0x75, 0x2D);
            draw_colour_set(0);
            e = GENS(waveform) + 0x32;
            dw = (int)(0x75 - U16(text_width(TXP(e)))) / 2;
            text_draw(U16(cx + dw + TEXT_DX(UI_TEXT(e))), U16(cy + TEXT_DY(UI_TEXT(e)) + 0xF), TXP(e));
        }
    }

    draw_colour_set(0);                                        /* status bar */
    cy = h + y;
    rect_fill(x, U16(cy - 0x21), w, 3);
    draw_colour_set(0x282828);
    rect_fill(x, U16(cy - 0x1E), w, 0x1E);
    draw_colour_set(0xBBBBBB);
    {
        u32 e = GENS(waveform) + 0x32;
        gs_text(x + TEXT_DX(UI_TEXT(e)) + 0x19, cy + TEXT_DY(UI_TEXT(e)) - 0x17, TXP(e));
    }
}
