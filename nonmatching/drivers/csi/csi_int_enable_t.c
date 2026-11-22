/*
 * csi_int_enable_t -- stock 0x40065EA4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_int_enable.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_int_enable_t(u32 sel, u32 ch, u32 bits) { csi_int_enable(sel, ch, bits); }
