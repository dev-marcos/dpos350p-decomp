/*
 * gic_dist_init -- stock 0x400669E0 (304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GIC distributor: off, target all SPIs to this CPU, level/edge config 0, priority 0xA0, disable all, on. Line count from GICD_TYPER (max 0x3FC).
 */
#include "hw/platform.h"

void gic_dist_init(u32 base)
{
    u32 dist = base + 0x1000, lines, i, cpumask;
    gic_writel(dist, 0);
    lines = ((gic_readl(base + 0x1004) & 0x1F) + 1) * 32;
    if (lines > 0x3FC) lines = 0x3FC;
    cpumask = 1u << gic_cpu_id();
    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;
    for (i = 32; i < lines; i += 4)
        gic_writel(base + 0x1800 + i, cpumask);
    for (i = 32; i < lines; i += 16)
        gic_writel(base + 0x1C00 + i / 4, 0);
    for (i = 32; i < lines; i += 4)
        gic_writel(base + 0x1400 + i, 0xA0A0A0A0u);
    for (i = 32; i < lines; i += 32)
        gic_writel(base + 0x1180 + i / 8, 0xFFFFFFFFu);
    gic_writel(dist, 1);
}
