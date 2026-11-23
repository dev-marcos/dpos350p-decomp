/*
 * battery_icon_draw -- stock 0x4001C200 (324 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the battery icon in the rectangle at 0x42E6455E (x, y, w, h): black background, a 13 x 24 outline and the 5 x 2 terminal, a fill of 2 * level (SYS+0x778) pixels, and when SYS+0x779 is set a white charging mark (a bar and a 4-row triangle).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "sys_info.h"
#include "state/ui_windows.h"


void battery_icon_draw(void)
{
    u32 x = WREC(4, x), y = WREC(4, y);
    u32 w = WREC(4, w), h = WREC(4, h), lv;
    draw_colour_set(0);
    rect_fill(x, y, w, h);
    draw_colour_set(0xDDDDDD);
    rect_outline_draw(x, (unsigned short)(y + 2), 13, 24);
    rect_fill((unsigned short)(x + 4), y, 5, 2);
    draw_colour_set(0x88FF);
    lv = SI(battery_level);
    rect_fill((unsigned short)(x + 2), (unsigned short)(y + 24 - 2 * lv), 9, 2 * lv);
    if (SI(charging) == 0)
        return;
    draw_colour_set(0xFFFFFF);
    rect_fill((unsigned short)(x + 5), (unsigned short)(y + 8), 3, 10);
    func_4002ecc4_span((unsigned short)(y + 0x11), (unsigned short)(x + 3), (unsigned short)(x + 9));
    func_4002ecc4_span((unsigned short)(y + 0x12), (unsigned short)(x + 4), (unsigned short)(x + 8));
    func_4002ecc4_span((unsigned short)(y + 0x13), (unsigned short)(x + 5), (unsigned short)(x + 7));
    func_4002ecc4_span((unsigned short)(y + 0x14), (unsigned short)(x + 6), (unsigned short)(x + 6));
}
