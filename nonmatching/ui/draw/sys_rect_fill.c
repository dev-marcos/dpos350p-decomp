/*
 * sys_rect_fill -- stock 0x4002C5C4 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Colour 0 (0x4002EA38), then 0x4002ECC4 with the four u16 at SYS+4, +6, +8, +0xA.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "sys_info.h"

void sys_rect_fill(void)
{
    draw_colour_set(0);
    rect_fill(SI(plot_x0), SI(plot_y0), SI(plot_width), SI(plot_height));
}
