/*
 * mmc_check_range -- stock 0x40067678 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * start + count - 1 must not exceed the card capacity (sectors); else two messages and error 0xFFFCA9A3.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"
#include "shared/drivers.h"

int mmc_check_range(u32 start, u32 count)
{
    u32 end = start + count - 1;
    if (mmc_capacity >= end)
        return 0;
    dbg_printf(STR(4008cc4c));
    dbg_printf(STR(4008cd70), mmc_capacity);
    return (int)0xFFFCA9A3u;
}
