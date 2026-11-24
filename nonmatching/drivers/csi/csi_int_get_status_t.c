/*
 * csi_int_get_status_t -- stock 0x40065EA8 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_int_get_status.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_int_get_status_t(u32 sel, u32 ch, unsigned char *st) { csi_int_get_status(sel, ch, st); }
