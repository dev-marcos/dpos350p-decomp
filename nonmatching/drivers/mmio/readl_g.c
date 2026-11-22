/*
 * readl_g -- stock 0x40066C80 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register read.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"

u32 readl_g(u32 addr) { return REG32(addr); }
