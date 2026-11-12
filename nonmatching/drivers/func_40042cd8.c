/*
 * avs_init -- stock 0x40042CD8 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * AVS counters: divider 0 = 24000, divider 1 = 24.
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void avs0_set_div_default(void);
void avs1_set_div_default(void);

void avs_init(void)
{
    avs0_set_div_default();
    avs1_set_div_default();
}
