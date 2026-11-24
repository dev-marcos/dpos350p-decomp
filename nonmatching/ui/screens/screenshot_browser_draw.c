/*
 * screenshot_browser_draw -- stock 0x400250DC (3104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window 20 (layout 0x42E64520 + 0x11E): the saved-screenshot browser. Light grey (0xDDDDDD) body and a dark right edge; a 5 x 3 grid from page SYS16(0xF9E) x per-page SI(list_page_size): for each non-zero file number at SYS16(0xFA6 + 2 i) (a zero ends the grid), screenshot_path_make(n) (name at SYS+0x88C1), the THUMB_W x THUMB_H thumbnail k (SYS+0x12ED60 + 0x256D8 k, size u16 SYS+0x109684 / +0x109686) copied into the frame buffer, touch button 0x42E61FF8 + 0x728 + 10 k (203 x 138), a 9-line gradient, the centred name, and in selection mode (SYS+0xF90) a dark veil with a check box filled when SYS+0x5DC6 + i is set. Title bar (0x202020, 30 high): image 0x4008A038, text entry 110, then entries 119 / 120 / 121 / 122 / 123 with the counts SYS16(0xFA0), SYS16(0xFA2) and the page SYS16(0xF9E) + 1. Bottom bar at y = 540 - window y: six 170 x 60 touch buttons (+0x7BE + 10 i, pressed ones in theme colour SYS+0x3DD54C) labelled by entries 67, 113..117.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "util.h"
#include "sys_info.h"
#include "state/ui_texts.h"
#include "state/theme.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void screenshot_path_make_raw(u32 v) __asm__("screenshot_path_make");

#define U16(v)    ((v) & 0xFFFFu)
#define LAY16(o)  (*(volatile unsigned short *)(0x42E64520u + (o)))
#define BR8(o)    (*(volatile unsigned char *)(0x40544E68u + 0xF90u + (o)))
#define BR16(o)   (*(volatile unsigned short *)(0x40544E68u + 0xF90u + (o)))
#define BRS16(o)  (*(volatile short *)(0x40544E68u + 0xF90u + (o)))
#define TXE(k)    (0x42C2FE74u + (k) * 0x1A0u)
#define TXP(k)    (*(const char *volatile *)TXE(k))
#define TEXT_W    (*(volatile unsigned short *)0x42E68BE4u)
#define TREC(o)   ((volatile unsigned char *)(0x42E61FF8u + (o)))
#define NAME      ((const char *)(0x40544E68u + 0x88C1u))
#define THUMB_W   (*(volatile unsigned short *)(0x40544E68u + 0x109684u))
#define THUMB_H   (*(volatile unsigned short *)(0x40544E68u + 0x109686u))
#define FB_PTR    (*(volatile u32 *)0x40095498u)

/* one text-table entry at (x + dx, y + dy) with a 1 x 2 shadow */
static void tb_label(u32 x, u32 y, u32 k)
{
    text_shadow_draw(U16(x + TEXT_DX(UI_TEXT(k))), U16(y + TEXT_DY(UI_TEXT(k))), TXP(k), 1, 2, 0);
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
    text_shadow_draw(U16(x + TEXT_DX(UI_TEXT(title)) + 0x23), U16(y + TEXT_DY(UI_TEXT(title)) + 8), TXP(title), 1, 2, 0);
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E64A98u);
    draw_colour_set(0x777777);
    xa = x + 0x249;
    tb_label(xa, y + 7, 119);
    xa = xa + TEXT_W + TEXT_DX(UI_TEXT(119)) + 6;
    tb_number(xa, y + 7, BRS16(0x10));
    tb_label(xa + TEXT_W + 6, y + 7, 120);
    xb = x + 0x2DF;
    tb_label(xb, y + 7, 119);
    xb = xb + TEXT_W + TEXT_DX(UI_TEXT(119)) + 6;
    tb_number(xb, y + 7, BRS16(0x12));
    tb_label(xb + TEXT_W + 6, y + 7, 121);
    xa = x + 0x37A;
    tb_label(xa, y + 7, 123);
    xa = xa + TEXT_W + TEXT_DX(UI_TEXT(123));
    tb_label(xa, y + 7, 122);
    xa = xa + TEXT_W + TEXT_DX(UI_TEXT(122)) + 6;
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
            draw_colour_set(THEME_PRESSED);
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
                    draw_colour_set(THEME_OPTION_SELECTED);
                    rect_fill(U16(cx + 0x5A), U16(cy + 0x3A), 0x16, 0x16);
                }
            }
        }
    }
bars:
    tb_bars(x, y, w, img, title, bar_rec);
}

void screenshot_browser_draw(void)
{
    tb_draw(0x11E, screenshot_path_make_raw, 0x728, 0x4008A038u, 110, 0x7BE);
}
