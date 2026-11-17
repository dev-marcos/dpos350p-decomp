/*
 * rect_42e68c38_draw -- stock 0x40034284 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * u16 x, y, w, h at 0x42E68C38: 0x4003436C(x, y, (u16)(x + w + 1), (u16)(y + h + 1)).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003436c(u32 a, u32 b, u32 c, u32 d);

void rect_42e68c38_draw(void)
{
    volatile unsigned short *h = (volatile unsigned short *)0x42E68C38u;
    u32 x = h[0], y = h[1];
    func_4003436c(x, y, (h[2] + x + 1) & 0xFFFF, (h[3] + y + 1) & 0xFFFF);
}
