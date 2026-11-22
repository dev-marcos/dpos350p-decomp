/*
 * timer_start -- stock 0x40042C1C (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Timer n: CTRL |= 1.
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

void timer_start(u32 n)
{
    u32 ctrl = TMR_CTRL(n);
    writel_d(ctrl, readl_d(ctrl) | 1);
}
