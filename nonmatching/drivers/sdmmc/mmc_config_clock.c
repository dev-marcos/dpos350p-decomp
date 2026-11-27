/*
 * mmc_config_clock -- stock 0x4006797C (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Card clock off, module clock to hz, CLKCR divider 0, card clock on; message per failure (-1).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"
#include "shared/drivers.h"

int mmc_config_clock(u32 n, u32 hz)
{
    u32 base = mmc_base(n);
    if (mmc_clk_disable(n)) { dbg_printf(STR(4008e0a0)); return -1; }
    if (mmc_set_mod_clk(n, hz)) { dbg_printf(STR(4008e73c)); return -1; }
    writel_a(base + 4, readl_a(base + 4) & ~0xFFu);
    if (mmc_clk_enable(n)) { dbg_printf(STR(4008e754)); return -1; }
    dbg_printf(STR(4008e0bc));
    return 0;
}
