/*
 * csi_disable -- stock 0x400658EC (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSI_EN bit 0 off.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_disable(u32 sel) { csi_clrbits(csi_base_tbl[sel], 1); }
