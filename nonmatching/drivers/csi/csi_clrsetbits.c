/*
 * csi_clrsetbits -- stock 0x40065894 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * (v & ~clr) | set.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_clrsetbits(u32 addr, u32 clr, u32 set) { csi_writel(addr, set | (csi_readl(addr) & ~clr)); }
