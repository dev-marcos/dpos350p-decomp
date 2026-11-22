/*
 * avs1_set_div_default -- stock 0x40042D4C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * avs_set_div(1, 24) (the stock code falls through into avs_set_div).
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void avs_set_div(u32 idx, u32 div);

void avs1_set_div_default(void)
{
    avs_set_div(1, 0x18);
}
