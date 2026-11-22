/*
 * gic_init -- stock 0x40066B8C (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GIC at 0x01C80000, irq_base 0, 0x9C table entries all set to default_irq_handler; distributor and CPU interface init. (Previously listed as irq_disable.)
 */
#include "hw/platform.h"
#include "drivers/gic.h"

void gic_init(void)
{
    int i;
    g_gic.base_lo = 0x01C80000u;
    g_gic.base_hi = 0;
    g_gic.irq_base = 0;
    g_gic.nr = 0x9C;
    for (i = 0; i < 0x9C; i++)
        irq_table[i] = default_irq_handler;
    gic_dist_init(g_gic.base_lo);
    gic_cpu_init(g_gic.base_lo);
}
