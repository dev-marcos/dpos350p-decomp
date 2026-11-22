/*
 * delay_us -- stock 0x40042D0C (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Busy-wait using AVS counter 1 (restart, then poll until 2*CNT1 >= n).
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

void delay_us(u32 n)
{
    avs1_restart();
    while (avs1_read() < n)
        ;
}
