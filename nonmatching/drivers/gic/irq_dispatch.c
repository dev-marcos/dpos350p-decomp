/*
 * irq_dispatch -- stock 0x400668E4 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Read GICC_IAR (+0x200C), call the handler of (id - irq_base) when it is in range, then write the id to GICC_EOIR (+0x2010).
 */
#include "hw/platform.h"

void irq_dispatch(void)
{
    u32 id = gic_readl(g_gic.base_lo + 0x200C) & 0x3FF;
    int n = (int)id - g_gic.irq_base;
    if (n < 0 || n >= g_gic.nr)
        return;
    irq_table[n]();
    gic_writel(g_gic.base_lo + 0x2010, id);
}
