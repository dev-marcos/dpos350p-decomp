/*
 * mmc_update_clk -- stock 0x4006773C (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CMD = 0x80202000 (update clock only, wait previous data), wait for the start bit to clear (200000 x 1 us).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"
#include "shared/drivers.h"

int mmc_update_clk(u32 n)
{
    u32 base = mmc_base(n), t = 200000;
    writel_a(base + 0x18, 0x80202000u);
    while (readl_a(base + 0x18) & 0x80000000u) {
        if (t-- == 0) {
            dbg_printf(STR(4008f83c));
            return -1;
        }
        mmc_delay_us(1);
    }
    return 0;
}
