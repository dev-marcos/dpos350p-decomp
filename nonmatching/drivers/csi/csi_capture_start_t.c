/*
 * csi_capture_start_t -- stock 0x40065EA0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_capture_start.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_capture_start_t(u32 sel, u32 ch_total, u32 mode) { csi_capture_start(sel, ch_total, mode); }
