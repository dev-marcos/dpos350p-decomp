/*
 * wdt_system_reset -- stock 0x40067574 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Watchdog 0: configure (interval 0, whole-system reset, no IRQ) and enable.
 */
#include "hw/drivers.h"
#include "drivers/wdt.h"

void wdt_system_reset(void)
{
    wdt_config(0, 1, 0);
    wdt_enable();
}
