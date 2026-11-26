/*
 * message_box_erase -- stock 0x40034284 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * u16 x, y, w, h at 0x42E68C38: 0x4003436C(x, y, (u16)(x + w + 1), (u16)(y + h + 1)).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void message_box_erase(void)
{
    volatile unsigned short *h = (volatile unsigned short *)0x42E68C38u;
    u32 x = h[0], y = h[1];
    fb_rect_restore(x, y, (h[2] + x + 1) & 0xFFFF, (h[3] + y + 1) & 0xFFFF);
}
