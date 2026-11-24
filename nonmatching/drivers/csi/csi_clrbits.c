/*
 * csi_clrbits -- stock 0x40065864 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Read, AND NOT, write.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_clrbits(u32 addr, u32 bits) { csi_writel(addr, csi_readl(addr) & ~bits); }
