/*
 * csi_clear_frame_flag -- stock 0x40066108 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * mov r0, #0 then falls into csi_set_frame_flag.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_clear_frame_flag(void) { csi_set_frame_flag(0); }
