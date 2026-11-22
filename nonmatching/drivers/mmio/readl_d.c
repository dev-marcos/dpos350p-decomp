/*
 * readl_d -- stock 0x40042B50 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register read (timer source file copy).
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

u32 readl_d(u32 addr) { return REG32(addr); }
