/*
 * ui_colour_buttons_draw -- stock 0x40028084 (664 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Column of four 64 x 150 buttons at window rectangle 0x42E64520+0x18E (x + 2), each with a shadowed alpha image (0x400860E4, 0x400863DC, 0x400866D4, 0x400869CC) in its own colour (0xFF2288, 0x88FF22, 0xFF8822, 0x2288FF) and touch slots 0x42E61FF8+0x9F8/0xA02/0xA0C/0xA16. A slot whose byte +1 is exactly 1 is filled with colour SYS+0x3DD54C (pressed). Background colour SYS+0x3DD53C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "shared/access.h"
#include "state/theme.h"

#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define TXE(k) (0x42C2FE74u + 0x1A0u * (k))
#define TXS(k) ((const char *)W32(TXE(k)))
#define DX(k) (*(volatile signed char *)(TXE(k) + 0x194))
#define DY(k) (*(volatile signed char *)(TXE(k) + 0x195))
#define U16(v) ((unsigned short)(v))

void ui_colour_buttons_draw(void)
{
    IMG_DECL(400860e4); IMG_DECL(400863dc); IMG_DECL(400866d4); IMG_DECL(400869cc);
    static const u32 img[4] = {(u32)d_400860e4, (u32)d_400863dc, (u32)d_400866d4, (u32)d_400869cc};
    static const u32 ink[4] = {0xFF2288u, 0x88FF22u, 0xFF8822u, 0x2288FFu};
    u32 x = U16(WR16(0x18E) + 2), y = WR16(0x190), w = WR16(0x192), h = WR16(0x194), i;
    u32 tx = x + 0x14;
    draw_colour_set(THEME_WORD(0x0C));
    rect_fill(x, y, w, h);
    for (i = 0; i < 4; i++) {
        if (i >= 2) {
            draw_colour_set(THEME_WORD(0x0C));
            rect_fill(x, U16(y), 0x40, 0x97);
        }
        draw_colour_set(ink[i]);
        draw_text_shadowed(U16(tx), U16(y + 0x3D), img[i], 2, 2, 0);
        touch_button_set_rect(x, U16(y), 0x40, 0x96, BTN + 0x9F8 + 10 * i);
        if (BTN[0x9F9 + 10 * i] == 1) {
            draw_colour_set(THEME_PRESSED);
            rect_fill(x, U16(y), 0x40, 0x96);
        }
        y += 0x96;
    }
}
