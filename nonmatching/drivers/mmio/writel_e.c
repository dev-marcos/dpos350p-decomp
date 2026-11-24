/*
 * writel_e -- stock 0x400427A8 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register write (CCU source file copy).
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"
#include "shared/drivers.h"

void writel_e(u32 addr, u32 v) { REG32(addr) = v; }
