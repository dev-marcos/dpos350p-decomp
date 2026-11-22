/*
 * irq_register -- stock 0x40066BD4 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Install handler, set the priority byte (prio << 3) in GICD_IPRIORITYR, enable or disable the interrupt.
 */
#include "hw/platform.h"

void irq_register(int irq, void (*handler)(void), int prio, int enable)
{
    u32 reg, v;
    int sh;
    dbg_printf(STR(0x4008E724));
    reg = g_gic.base_lo + (u32)((irq / 4) * 4) + 0x1400;
    sh = (irq - (irq / 4) * 4) * 8;
    v = gic_readl(reg);
    gic_writel(reg, (v & ~(0xFFu << sh)) | ((u32)prio << 3 << sh));
    irq_table[irq] = handler;
    if (enable)
        gic_enable_irq(&g_gic, irq);
    else
        gic_disable_irq(&g_gic, irq);
}
