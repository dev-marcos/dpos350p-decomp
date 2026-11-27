/*
 * pixel_put -- stock 0x4002EB70 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Pixel (x, y): when byte 0x42E6865C is set, 0x4002EA88(x, y) (the blended path); else the u16 at 0x42E68660 is stored at target + 2 * (x + 1022 * y), target = *(u32 *)0x40095498.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void pixel_put(u32 x, u32 y)
{
    if (*(volatile unsigned char *)0x42E6865Cu) {
        pixel_blend(x, y);
        return;
    }
    *(volatile unsigned short *)(*(volatile u32 *)IMG(40095498) + (x + y * 1022u) * 2u) =
        *(volatile unsigned short *)0x42E68660u;
}
