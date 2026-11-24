/*
 * csi_readl -- stock 0x4006582C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register read.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

u32 csi_readl(u32 addr) { return REG32(addr); }
