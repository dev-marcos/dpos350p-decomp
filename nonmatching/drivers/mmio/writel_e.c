/*
 * writel_e -- stock 0x400427A8 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register write (CCU source file copy).
 */
#include "hw/drivers.h"

u32 readl_e(u32 addr);
void writel_e(u32 addr, u32 v);
#define CCU 0x01C20000u
#define PLL_LOCK 0x10000000u

void writel_e(u32 addr, u32 v) { REG32(addr) = v; }
