/*
 * delay_ms -- stock 0x40042CF0 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Busy-wait using AVS counter 0 (restart, then poll until 2*CNT0 >= n). Unit DERIVED from the 24000 divider.
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void avs0_restart(void);
u32 avs0_read(void);

void delay_ms(u32 n)
{
    avs0_restart();
    while (avs0_read() < n)
        ;
}
