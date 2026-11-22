/*
 * timer_setup -- stock 0x40042B60 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Timer n: CTRL = 0, two read-back writes, set clock source (bit 2) and reload (bit 1), INTV = 24*interval, optionally enable its IRQ.
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

void timer_setup(u32 n, u32 interval, u32 irq)
{
    u32 ctrl = TMR_CTRL(n);
    writel_d(ctrl, 0);
    writel_d(ctrl, readl_d(ctrl));
    writel_d(ctrl, readl_d(ctrl));
    writel_d(ctrl, readl_d(ctrl) | 4);
    writel_d(ctrl, readl_d(ctrl) | 2);
    writel_d(TMR_INTV(n), interval * 24);
    if (irq == 1)
        writel_d(TMR_IRQ_EN, readl_d(TMR_IRQ_EN) | lsl32(1, n));
}
