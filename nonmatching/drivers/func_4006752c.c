/*
 * wdt_irq_ack -- stock 0x4006752C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Watchdog 0: if IRQ_STA bit 0 is set, write it back (ack) and return 1, else 0.
 */
#include "hw/drivers.h"

int wdt_irq_ack(void)
{
    if (readl_b(WDOG0_IRQ_STA) & 1) {
        writel_b(WDOG0_IRQ_STA, readl_b(WDOG0_IRQ_STA) | 1);
        return 1;
    }
    return 0;
}
