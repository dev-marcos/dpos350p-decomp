/*
 * avs0_read -- stock 0x40042E10 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 2 * AVS_CNT0.
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"

u32 avs0_read(void) { return readl_d(AVS_CNT0) << 1; }
