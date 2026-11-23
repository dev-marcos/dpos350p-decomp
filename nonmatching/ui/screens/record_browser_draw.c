/*
 * record_browser_draw -- stock 0x40027498 (3044 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window 26 (layout + 0x172): the saved-record browser. Same frame, title bar (image 0x4008A318, text entry 112) and bottom bar (buttons +0x9B2 + 10 i) as screenshot_browser_draw, with font 0x42E658D8 for the grid. Each cell: number_text_88c0(n) (name at SYS+0x88C0) and list_c_record_preview(n), a 0xBBBBBB box, and unless the point count u16 SYS+0x109684 is 3..6 a line plot of the points u16 SYS+0x10968E[i] (vertical segments between neighbours at x = cell + i + 8, black or 0x555555 in selection mode); touch button +0x91C + 10 k, the centred name, the selection veil (0xBB000000, check box 0x333333), and a SET tag in theme colour +0x14 on the cell whose number is u16 SYS+0x3DA52E.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "util.h"
#include "state/ui_windows.h"

#define U16(v)    ((v) & 0xFFFFu)
#define LAY16(o)  (*(volatile unsigned short *)(0x42E64520u + (o)))
#define BR8(o)    (*(volatile unsigned char *)(0x40544E68u + 0xF90u + (o)))
#define BR16(o)   (*(volatile unsigned short *)(0x40544E68u + 0xF90u + (o)))
#define BRS16(o)  (*(volatile short *)(0x40544E68u + 0xF90u + (o)))
#define TXE(k)    (0x42C2FE74u + (k) * 0x1A0u)
#define TXP(k)    (*(const char *volatile *)TXE(k))
#define TDX(k)    (*(volatile signed char *)(TXE(k) + 0x194))
#define TDY(k)    (*(volatile signed char *)(TXE(k) + 0x195))
#define TEXT_W    (*(volatile unsigned short *)0x42E68BE4u)
#define TREC(o)   ((volatile unsigned char *)(0x42E61FF8u + (o)))
#define THEME(o)  (*(volatile u32 *)(0x40544E68u + 0x3DD54Cu + (o)))
#define NAME      ((const char *)(0x40544E68u + 0x88C1u))
#define THUMB_W   (*(volatile unsigned short *)(0x40544E68u + 0x109684u))
#define THUMB_H   (*(volatile unsigned short *)(0x40544E68u + 0x109686u))
#define FB_PTR    (*(volatile u32 *)0x40095498u)

/* one text-table entry at (x + dx, y + dy) with a 1 x 2 shadow */
static void tb_label(u32 x, u32 y, u32 k)
{
    text_shadow_draw(U16(x + TDX(k)), U16(y + TDY(k)), TXP(k), 1, 2, 0);
}

static void tb_number(u32 x, u32 y, int v)
{
    text_d6f8_shadow_draw(U16(x), U16(y), (u32)v, (u32)(v >> 31), 0, 1, 2, 0);
}

/* light grey body with a dark right edge */
static void tb_frame(u32 x, u32 y, u32 w, u32 h, u32 font)
{
    draw_colour_set(0xDDDDDD);
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    vline_draw(U16(x + w - 1), y, U16(h + y));
    half_42e68be2_set(1);
    word_42e68c74_set(font);
}

/* title bar (image, title entry, count / page fields) and the bottom bar of six buttons */
static void tb_bars(u32 x, u32 y, u32 w, u32 img, u32 title, u32 bar_rec)
{
    u32 i, cx, cy, xa, xb;
    /* title bar: image, title, "count", "page n / m" */
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x202020);
    rect_fill(x, y, w, 0x1E);
    draw_colour_set(0xBBBBBB);
    draw_text_shadowed(U16(x + 5), U16(y + 5), img, 2, 2, 0);   /* stock: last argument is an unset stack byte */
    text_shadow_draw(U16(x + TDX(title) + 0x23), U16(y + TDY(title) + 8), TXP(title), 1, 2, 0);
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E64A98u);
    draw_colour_set(0x777777);
    xa = x + 0x249;
    tb_label(xa, y + 7, 119);
    xa = xa + TEXT_W + TDX(119) + 6;
    tb_number(xa, y + 7, BRS16(0x10));
    tb_label(xa + TEXT_W + 6, y + 7, 120);
    xb = x + 0x2DF;
    tb_label(xb, y + 7, 119);
    xb = xb + TEXT_W + TDX(119) + 6;
    tb_number(xb, y + 7, BRS16(0x12));
    tb_label(xb + TEXT_W + 6, y + 7, 121);
    xa = x + 0x37A;
    tb_label(xa, y + 7, 123);
    xa = xa + TEXT_W + TDX(123);
    tb_label(xa, y + 7, 122);
    xa = xa + TEXT_W + TDX(122) + 6;
    tb_number(xa, y + 7, BRS16(0xE) + 1);
    draw_colour_set(0);
    rect_fill(U16(x + 0x2C4), U16(y + 7), 2, 0xF);
    rect_fill(U16(x + 0x35F), U16(y + 7), 2, 0xF);

    /* bottom bar: six 170 x 60 buttons with separators and labels */
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0x202020);
    cy = 0x21C - y;
    rect_fill(x, U16(cy), w, 0x3C);
    draw_colour_set(0);
    for (i = 1; i <= 5; i++)
        rect_fill(U16(x + 0xAA * i), U16(cy + 0xF), 2, 0x1E);
    for (i = 0; i < 6; i++) {
        cx = x + 0xAA * i;
        touch_button_set_rect(U16(cx), U16(cy), 0xAA, 0x3C, TREC(bar_rec + 10 * i));
        if (TREC(bar_rec + 10 * i)[1]) {
            draw_colour_set(THEME(0));
            if (i == 0)
                rect_fill(U16(cx), U16(cy), 0xAA, 0x3C);
            else
                rect_fill(U16(cx + 2), U16(cy), 0xA8, 0x3C);
        }
    }
    draw_colour_set(0xAAAAAA);
    for (i = 0; i < 6; i++) {
        u32 kk = i ? 112 + i : 67;
        cx = x + 0xAA * i;
        tb_label(cx + ((0xA9 - text_width(TXP(kk))) >> 1), cy + 0x16, kk);
    }
}

static void tb_draw(u32 lay, void (*path)(u32), u32 grid_rec, u32 img, u32 title, u32 bar_rec)
{
    u32 x = LAY16(lay), y = LAY16(lay + 2), w = LAY16(lay + 4), h = LAY16(lay + 6);
    u32 row, col, k = 0, idx, cx, cy, i;
    volatile unsigned short *item;
    volatile unsigned char *sel;

    tb_frame(x, y, w, h, 0x42E65418u);
    idx = (u32)(BRS16(0xE) * BR16(0xA));
    cy = 0x2D;
    for (row = 0; row < 3; row++, cy += 0xA5) {
        item = (volatile unsigned short *)(0x40544E68u + 0xFA6u + 2 * idx);
        sel = (volatile unsigned char *)(0x40544E68u + 0x5DC6u + idx);
        cx = x;
        for (col = 0; col < 5; col++, cx += 0xCC, idx++, k++) {
            u32 dst, src;
            if (!item[col])
                goto bars;
            path(item[col]);
            /* thumbnail k of the page: THUMB_W x THUMB_H pixels into the frame buffer */
            dst = 2 * (row * 0x292B6u + x + 0xCC * col);
            src = 0x40544E68u + 0x256D8u * U16(k) + 0x12ED60u;
            for (i = 0; i < THUMB_H; i++, dst += 0x7FC)
                copy_u16((unsigned short *)(FB_PTR + dst + 0x1674C), (const unsigned short *)(src + 2 * THUMB_W * i),
                         THUMB_W);
            touch_button_set_rect(U16(cx), U16(cy), 0xCB, 0x8A, TREC(10 * U16(k) + grid_rec));
            for (i = 1; i < 10; i++) {
                draw_colour_set(0x111111 * i);
                hspan_fill(U16(cy + 0x8A + i - 1), U16(cx), U16(cx + 0xCA));
            }
            draw_colour_set(0);
            text_draw(U16(cx + ((0xCB - text_width(NAME)) >> 1)), U16(cy + 0x91), NAME);
            if (BR8(0)) {                              /* selection mode */
                draw_colour_set(0x88000000u);
                rect_fill(U16(cx), U16(cy), 0xCB, 0x8A);
                draw_colour_set(0x666666);
                rect_outline_draw(U16(cx + 0x56), U16(cy + 0x36), 0x1E, 0x1E);
                rect_outline_draw(U16(cx + 0x55), U16(cy + 0x35), 0x20, 0x20);
                if (sel[col]) {
                    draw_colour_set(THEME(0x14));
                    rect_fill(U16(cx + 0x5A), U16(cy + 0x3A), 0x16, 0x16);
                }
            }
        }
    }
bars:
    tb_bars(x, y, w, img, title, bar_rec);
}

#define NAME_C    ((const char *)(0x40544E68u + 0x88C0u))
#define PLOT(i)   (*(volatile unsigned short *)(0x40544E68u + 0x10968Eu + 2 * (i)))
#define CUR_REC   (*(volatile unsigned short *)(0x40544E68u + 0x3DA52Eu))

void record_browser_draw(void)
{
    u32 x = WREC(26, x), y = WREC(26, y), w = WREC(26, w), h = WREC(26, h);
    u32 row, col, k = 0, idx, cx, cy, j, n;
    volatile unsigned short *item;
    volatile unsigned char *sel;

    tb_frame(x, y, w, h, 0x42E658D8u);
    idx = (u32)(BRS16(0xE) * BR16(0xA));
    cy = 0x2D;
    for (row = 0; row < 3; row++, cy += 0xA5) {
        item = (volatile unsigned short *)(0x40544E68u + 0xFA6u + 2 * idx);
        sel = (volatile unsigned char *)(0x40544E68u + 0x5DC6u + idx);
        cx = x;
        for (col = 0; col < 5; col++, cx += 0xCC, idx++, k++) {
            n = item[col];
            if (!n)
                goto bars;
            number_text_88c0(n);
            list_c_record_preview(n);
            draw_colour_set(0xBBBBBB);
            rect_fill(U16(cx), U16(cy), 0xCB, 0x8A);
            draw_colour_set(BR8(0) ? 0x555555 : 0);
            if ((u32)(THUMB_W - 3) >= 4)
                for (j = 3; j < (u32)(THUMB_W - 3); j++) {
                    u32 a = PLOT(j - 3), b = PLOT(j - 2);
                    if (a < b)
                        vline_draw(U16(cx + j + 5), U16(a + cy), U16(b + cy));
                    else
                        vline_draw(U16(cx + j + 5), U16(b + cy), U16(a + cy));
                }
            touch_button_set_rect(U16(cx), U16(cy), 0xCB, 0x8A, TREC(10 * U16(k) + 0x91C));
            draw_colour_set(0);
            text_draw(U16(cx + ((0xCB - text_width(NAME_C)) >> 1)), U16(cy + 0x91), NAME_C);
            if (BR8(0)) {
                draw_colour_set(0xBB000000u);
                rect_fill(U16(cx), U16(cy), 0xCB, 0x8A);
                draw_colour_set(0xBBBBBB);
                rect_outline_draw(U16(cx + 0x56), U16(cy + 0x36), 0x1E, 0x1E);
                rect_outline_draw(U16(cx + 0x55), U16(cy + 0x35), 0x20, 0x20);
                if (sel[col]) {
                    draw_colour_set(0x333333);
                    rect_fill(U16(cx + 0x5A), U16(cy + 0x3A), 0x16, 0x16);
                }
            }
            if (n == CUR_REC) {
                draw_colour_set(THEME(0x14));
                rect_fill(U16(cx), U16(cy), 0x21, 0x14);
                draw_colour_set(0);
                text_draw(U16(cx + 4), U16(cy + 3), "SET");
            }
        }
    }
bars:
    tb_bars(x, y, w, 0x4008A318u, 112, 0x9B2);
}
