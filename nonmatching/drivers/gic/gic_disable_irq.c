/*
 * gic_disable_irq -- stock 0x40066994 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Set the bit of irq (+irq_base) in GICD_ICENABLER (base + 0x1180); armcc's signed n/32 and n%32 sequences kept.
 */
#include "hw/platform.h"
#include "drivers/gic.h"

void gic_disable_irq(gic_t *g, int irq)
{
    int n = irq + g->irq_base;
    u32 bias = (u32)(n >> 4) >> 27;              /* 31 for negative n */
    u32 reg = g->base_lo + ((u32)((int)(n + bias) >> 5) << 2) + 0x1180;
    gic_writel(reg, 1u << (n - (int)((n + bias) & 0xE0)));
}
