/*
 * trigger_level_drag_draw -- stock 0x40023D04 (340 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the window whose rectangle is at 0x42E64630 (x, y, w, h): background in colour word 0x409223B8, eight 0x555555 segments (line_draw) near the top and bottom, and registers the rectangle as touch button 0x42E62716.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "shared/access.h"

void trigger_level_drag_draw(void)
{
    u32 x = H16(0x42E64630u), y = H16(0x42E64632u), w = H16(0x42E64634u), h = H16(0x42E64636u), t, u;
    draw_colour_set(W32(0x409223B8u));
    rect_fill(x, y, w, h);
    draw_colour_set(0x555555);
    line_draw(x + 0xB, y + 0x19, x + 0x1F, y + 0xF);
    line_draw(x + 0x34, y + 0x19, x + 0x20, y + 0xF);
    line_draw(x + 0xB, y + 0x1A, x + 0x1F, y + 0x10);
    line_draw(x + 0x34, y + 0x1A, x + 0x20, y + 0x10);
    t = h + y - 0x1B;
    u = h + y - 0x11;
    line_draw(x + 0xB, t, x + 0x1F, u);
    line_draw(x + 0x34, t, x + 0x20, u);
    t = h + y - 0x1C;
    u = h + y - 0x12;
    line_draw(x + 0xB, t, x + 0x1F, u);
    line_draw(x + 0x34, t, x + 0x20, u);
    touch_button_set_rect(x, y, w, h, (volatile unsigned char *)0x42E62716u);
}
