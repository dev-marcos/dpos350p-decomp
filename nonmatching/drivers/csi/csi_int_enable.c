/*
 * csi_int_enable -- stock 0x40065BB8 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel INT_EN (+0x70) |= bits.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_int_enable(u32 sel, u32 ch, u32 bits) { csi_setbits(csi_base_tbl[sel] + (ch << 8) + 0x70, bits); }
