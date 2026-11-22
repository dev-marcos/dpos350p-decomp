/*
 * delay_ms -- stock 0x40042CF0 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Busy-wait using AVS counter 0 (restart, then poll until 2*CNT0 >= n). Unit DERIVED from the 24000 divider.
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

void delay_ms(u32 n)
{
    avs0_restart();
    while (avs0_read() < n)
        ;
}
