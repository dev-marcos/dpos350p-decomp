/*
 * wdt_enable -- stock 0x4006750C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Watchdog 0: MODE |= 1 (enable).
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"
#include "drivers/wdt.h"

void wdt_enable(void)
{
    writel_b(WDOG0_MODE, readl_b(WDOG0_MODE) | 1);
}
