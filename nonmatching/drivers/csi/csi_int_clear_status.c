/*
 * csi_int_clear_status -- stock 0x40065C50 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INT_STA (+0x74) = bits; the channel argument is ignored (channel 0 register).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_int_clear_status(u32 sel, u32 ch, u32 bits) { (void)ch; csi_writel_t(csi_base_tbl[sel] + 0x74, bits); }
