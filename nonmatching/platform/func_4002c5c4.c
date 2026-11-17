/*
 * sys_rect_fill -- stock 0x4002C5C4 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Colour 0 (0x4002EA38), then 0x4002ECC4 with the four u16 at SYS+4, +6, +8, +0xA.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void func_4002ecc4(u32 a, u32 b, u32 c, u32 d);

void sys_rect_fill(void)
{
    draw_colour_set(0);
    func_4002ecc4(SYS16(4), SYS16(6), SYS16(8), SYS16(0xA));
}
