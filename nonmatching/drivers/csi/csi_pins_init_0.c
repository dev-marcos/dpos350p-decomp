/*
 * csi_pins_init_0 -- stock 0x400660C0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * mov r0, #0 then falls into 0x400660C4 (b csi_pins_init).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_pins_init_0(void) { csi_pins_init(); }
