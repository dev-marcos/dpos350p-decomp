/*
 * gic_readl -- stock 0x400668D0 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register read (copy in the GIC source file).
 */
#include "hw/platform.h"

u32 gic_readl(u32 addr) { return REG32(addr); }
