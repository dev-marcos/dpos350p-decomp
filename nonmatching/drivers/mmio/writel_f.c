/*
 * writel_f -- stock 0x40042E84 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register write.
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"

void writel_f(u32 addr, u32 v) { REG32(addr) = v; }
