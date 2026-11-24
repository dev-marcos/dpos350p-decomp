/*
 * ui_usb_connect_screen -- stock 0x40029BC0 (760 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The 'USB connect' screen (text 11) in the window rectangle 0x42E64520+0x1C6: clears the frame buffer, a 0x252525 title bar with the image 0x40088EF8, a 0x252525 'Return' bar at the bottom (text 67, touch slot 0x42E61FF8+0x19C, drawn pressed when its byte +1 is set), then a fixed picture of grey boxes and the label 'USB' centred over x = 240..760. Colours from SYS+0x3DD54C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "shared/access.h"
#include "state/ui_texts.h"

#define WR16(o) H16(0x42E64520u + (o))
#define BTN ((volatile unsigned char *)0x42E61FF8u)
#define U16(v) ((unsigned short)(v))

#define COL(i) W32(0x40544E68u + 0x3DD54Cu + 4 * (i))

void ui_usb_connect_screen(void)
{
    u32 x = WR16(0x1C6), y = WR16(0x1C8), w = WR16(0x1CA), h = WR16(0x1CC), by, tw;
    const char *usb = "USB";
    fb_clear_current();
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(COL(1));
    draw_colour_set(0x252525);
    rect_fill(x, y, w, 0x1E);
    draw_colour_set(COL(2));
    draw_text_shadowed(U16(x + 5), U16(y + 5), 0x40088EF8u, 2, 2, 0);
    text_shadow_draw(U16(x + TEXT_DX(UI_TEXT(11)) + 0x23), U16(y + TEXT_DY(UI_TEXT(11)) + 8), TEXT_STR(UI_TEXT(11)), 1, 2, 0);
    by = y + h - 0x32;
    draw_colour_set(COL(1));
    draw_colour_set(0x252525);
    rect_fill(x, U16(by), w, 0x32);
    draw_colour_set(COL(2));
    tw = text_width(TEXT_STR(UI_TEXT(67)));
    text_shadow_draw(U16(x + ((w - tw) >> 1) + TEXT_DX(UI_TEXT(67))), U16(by + TEXT_DY(UI_TEXT(67)) + 0xF), TEXT_STR(UI_TEXT(67)), 1, 2, 0);
    touch_button_set_rect(x, U16(by), w, 0x32, BTN + 0x19C);
    if (BTN[0x19D]) {
        draw_colour_set(COL(0));
        rect_fill(x, U16(by), w, 0x32);
    }
    draw_colour_set(0xBBBBBB);
    rect_fill(0xF0, 0x69, 0x208, 0x140);
    hspan_fill(0x68, 0xF1, 0x2F6);
    hspan_fill(0x1A9, 0xF1, 0x2F6);
    draw_colour_set(0x999999);
    rect_fill(0xF0, 0x69, 0x208, 0x124);
    hspan_fill(0x68, 0xF1, 0x2F6);
    draw_colour_set(0x777777);
    rect_fill(0x1CC, 0x1AA, 0x50, 0x37);
    rect_fill(0x190, 0x1E1, 0xC8, 3);
    draw_colour_set(0x181818);
    rect_fill(0xF1, 0x6C, 0x206, 0x121);
    draw_colour_set(0xBBBBBB);
    tw = text_width(usb);
    text_shadow_draw(U16(0xF0 + ((0x208 - tw) >> 1)), 0xF2, usb, 1, 2, 0);
}
