/*
 * writel_d -- stock 0x40042B58 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register write (timer source file copy).
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"

void writel_d(u32 addr, u32 v) { REG32(addr) = v; }
