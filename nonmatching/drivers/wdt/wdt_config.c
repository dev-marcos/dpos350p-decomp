/*
 * wdt_config -- stock 0x40067488 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Watchdog 0: CFG[1:0] = cfg, MODE[7:4] = interval, IRQ_EN bit 0 = irq; then tail-calls wdt_irq_ack. Names DERIVED from the V3s register map.
 */
#include "hw/drivers.h"

int wdt_irq_ack(void);

int wdt_config(u32 interval, u32 cfg, u32 irq)
{
    writel_b(WDOG0_CFG, cfg | ((readl_b(WDOG0_CFG) >> 2) << 2));
    writel_b(WDOG0_MODE, (readl_b(WDOG0_MODE) & ~0xF0u) | (interval << 4));
    if (irq == 1)
        writel_b(WDOG0_IRQ_EN, readl_b(WDOG0_IRQ_EN) | 1);
    else
        writel_b(WDOG0_IRQ_EN, readl_b(WDOG0_IRQ_EN) & ~1u);
    return wdt_irq_ack();
}
