/*
 * csi_setbits -- stock 0x4006583C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Read, OR, write.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_setbits(u32 addr, u32 bits) { csi_writel(addr, bits | csi_readl(addr)); }
