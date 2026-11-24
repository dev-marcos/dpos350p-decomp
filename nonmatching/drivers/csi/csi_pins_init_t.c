/*
 * csi_pins_init_t -- stock 0x400660C4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_pins_init.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_pins_init_t(void) { csi_pins_init(); }
