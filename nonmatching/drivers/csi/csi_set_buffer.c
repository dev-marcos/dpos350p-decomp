/*
 * csi_set_buffer -- stock 0x40066030 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * BUF0 of channel 0 = addr.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_set_buffer(u32 addr) { csi_set_buffer_address(0, 0, 0, (u64)addr); }
