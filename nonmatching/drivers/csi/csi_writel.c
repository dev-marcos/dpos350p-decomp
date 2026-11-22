/*
 * csi_writel -- stock 0x40065834 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register write.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_writel(u32 addr, u32 v) { REG32(addr) = v; }
