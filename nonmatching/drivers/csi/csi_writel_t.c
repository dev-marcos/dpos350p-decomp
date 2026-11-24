/*
 * csi_writel_t -- stock 0x4006588C (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_writel (second entry used by tail calls).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_writel_t(u32 addr, u32 v) { csi_writel(addr, v); }
