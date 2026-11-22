/*
 * avs1_read -- stock 0x40042E2C (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 2 * AVS_CNT1.
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"

u32 avs1_read(void) { return readl_d(AVS_CNT1) << 1; }
