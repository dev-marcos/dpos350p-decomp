/*
 * touch_state_clear -- stock 0x40049A38 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the touch state block (0x42E68BB0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_state_clear(void)
{
    volatile u32 *w = (volatile u32 *)g_touch;
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    *(volatile unsigned short *)((u32)g_touch + 0x0C) = 0;
    *(volatile unsigned char *)((u32)g_touch + 0x0E) = 0;
    w[4] = 0;
}
