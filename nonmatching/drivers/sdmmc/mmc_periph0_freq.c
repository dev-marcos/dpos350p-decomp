/*
 * mmc_periph0_freq -- stock 0x400677A8 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PLL_PERIPH0 rate (copy in the SD source file): 24 MHz * N * K / 2.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"

#define mmc_capacity (*(u32 *)0x42E68CC0u)

s32 mmc_periph0_freq(void)
{
    u32 v = readl_a(0x01C20028u);
    s32 n = (s32)((v >> 8) & 0x1F) + 1, k = (s32)((v >> 4) & 3) + 1;
    s32 x = 24000000 * (s32)(short)(k * n);
    return (x + (s32)((u32)x >> 31)) >> 1;
}
