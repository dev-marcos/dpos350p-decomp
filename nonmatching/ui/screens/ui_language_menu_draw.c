/*
 * ui_language_menu_draw -- stock 0x400292E4 (900 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Language menu in the window rectangle 0x42E64520+0x84: four rows 60 pixels apart, each with the image 0x400891D8 and the name of a language (the strings at 0x40090430, 0x40090438, 0x4008E7B4, 0x4008FF6C: Chinese, 'English', Russian, 'Portugues' with accent), touch slots 0x42E61FF8+0x156 + 10 * i. A row is drawn selected when its button byte +1 is set or when the byte SYS+0x7AC (menu language, 1..4) is i + 1. Colours from SYS+0x3DD550.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "shared/access.h"
#include "state/theme.h"

#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define TXE(k) (0x42C2FE74u + 0x1A0u * (k))
#define TXS(k) ((const char *)W32(TXE(k)))
#define DX(k) (*(volatile signed char *)(TXE(k) + 0x194))
#define DY(k) (*(volatile signed char *)(TXE(k) + 0x195))
#define U16(v) ((unsigned short)(v))

void ui_language_menu_draw(void)
{
    IMG_DECL(40090430); IMG_DECL(40090438); IMG_DECL(4008e7b4); IMG_DECL(4008ff6c);
    static const u32 name[4] = {(u32)d_40090430, (u32)d_40090438, (u32)d_4008e7b4, (u32)d_4008ff6c};
    u32 x = WR16(0x84), y = WR16(0x86), w = WR16(0x88), h = WR16(0x8A), bx = x + 2, row, i;
    font_set(0x42E658D8u);
    text_spacing_set(1);
    draw_colour_set(THEME_MENU_BG);
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    for (i = 0, row = y + 0x14; i < 4; i++, row += 0x3C) {
        draw_colour_set(THEME_MENU_TEXT);
        draw_text_shadowed(U16(bx + 0x14), U16(row), IMGU(400891d8), 2, 2, 0);
        draw_colour_set(THEME_MENU_TEXT);
        text_shadow_draw(U16(bx + 0x41), U16(row + 3), (const char *)name[i], 1, 2, 0);
        touch_button_set_rect(U16(bx), U16(row - 0x14), w, 0x3C, BTN + 0x156 + 10 * i);
        if (BTN[0x157 + 10 * i] || *(volatile unsigned char *)(0x40544E68u + 0x7AC) == i + 1) {
            draw_colour_set(THEME_MENU_ROW_PRESSED);
            rect_fill(U16(bx), U16(row - 0x14), w, 0x3C);
        }
    }
}
