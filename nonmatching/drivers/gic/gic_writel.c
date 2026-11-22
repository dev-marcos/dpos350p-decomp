/*
 * gic_writel -- stock 0x400668D8 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register write.
 */
#include "hw/platform.h"
#include "drivers/gic.h"

void gic_writel(u32 addr, u32 v) { REG32(addr) = v; }
