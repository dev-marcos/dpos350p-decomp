/*
 * mmc_delay_ms -- stock 0x400675AC (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b delay_ms.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/timer.h"
#include "shared/drivers.h"

void mmc_delay_ms(u32 ms) { delay_ms(ms); }
