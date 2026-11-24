/*
 * periph0_freq -- stock 0x400428AC (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU: PLL_PERIPH0 output = 24 MHz * N * K / 2 (signed halving).
 */
#include "hw/drivers.h"
#include "drivers/ccu.h"
#include "drivers/mmio.h"
#include "shared/drivers.h"

s32 periph0_freq(void)
{
    u32 v = readl_e(CCU + 0x28);
    s32 n = (s32)((v >> 8) & 0x1F) + 1, k = (s32)((v >> 4) & 3) + 1;
    s32 x = 24000000 * (s32)(short)(k * n);
    return (x + (s32)((u32)x >> 31)) >> 1;
}
