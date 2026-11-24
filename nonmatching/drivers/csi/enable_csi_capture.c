/*
 * enable_csi_capture -- stock 0x400660AC (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Start capture on all configured channels in mode 1 (single).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void enable_csi_capture(void) { csi_capture_start_t(0, *(u32 *)(0x42E5549Cu + 0x10), 1); }
