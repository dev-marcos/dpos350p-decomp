/*
 * timers_init -- stock 0x40042D28 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * timer0_init, then avs_init.
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

void timers_init(void)
{
    timer0_init();
    avs_init();
}
