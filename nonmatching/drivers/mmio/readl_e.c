/*
 * readl_e -- stock 0x400427A0 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register read (CCU source file copy).
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"

#define CCU 0x01C20000u
#define PLL_LOCK 0x10000000u

u32 readl_e(u32 addr) { return REG32(addr); }
