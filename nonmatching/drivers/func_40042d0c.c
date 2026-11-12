/*
 * delay_us -- stock 0x40042D0C (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Busy-wait using AVS counter 1 (restart, then poll until 2*CNT1 >= n).
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void avs1_restart(void);
u32 avs1_read(void);

void delay_us(u32 n)
{
    avs1_restart();
    while (avs1_read() < n)
        ;
}
