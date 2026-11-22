/*
 * avs0_set_div_default -- stock 0x40042D40 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * avs_set_div(0, 24000).
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void avs_set_div(u32 idx, u32 div);

void avs0_set_div_default(void)
{
    avs_set_div(0, 0x5DC0);
}
