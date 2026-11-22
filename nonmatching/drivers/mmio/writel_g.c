/*
 * writel_g -- stock 0x40066C88 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register write.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"

void writel_g(u32 addr, u32 v) { REG32(addr) = v; }
