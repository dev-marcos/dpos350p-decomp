/*
 * gic_cpu_init -- stock 0x40066B10 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Banked SGI/PPI enables and priorities, priority mask 0xF8, CPU interface on.
 */
#include "hw/platform.h"

void gic_cpu_init(u32 base)
{
    int i;
    gic_writel(base + 0x1180, 0xFFFF0000u);
    gic_writel(base + 0x1100, 0x0000FFFFu);
    for (i = 0; i < 32; i += 4)
        gic_writel(base + 0x1400 + i, 0xA0A0A0A0u);
    gic_writel(base + 0x2004, 0xF8);
    gic_writel(base + 0x2000, 1);
}
