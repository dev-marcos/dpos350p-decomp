/*
 * mmc_delay_us -- stock 0x400675A8 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b delay_us.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/timer.h"

#define mmc_capacity (*(u32 *)0x42E68CC0u)

void mmc_delay_us(u32 us) { delay_us(us); }
