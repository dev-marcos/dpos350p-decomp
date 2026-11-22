/*
 * timers_init -- stock 0x40042D28 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * timer0_init, then avs_init.
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void timer0_init(void);
void avs_init(void);

void timers_init(void)
{
    timer0_init();
    avs_init();
}
