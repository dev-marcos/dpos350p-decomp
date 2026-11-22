/*
 * avs_set_div -- stock 0x40042D54 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Enable the AVS clock gate (CCU AVS_CLK bit 31) and set AVS_CNT_DIV[15:0] (idx 0) or [31:16] (idx 1) to div - 1.
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void avs_set_div(u32 idx, u32 div)
{
    writel_d(CCU_AVS_CLK, readl_d(CCU_AVS_CLK) | 0x80000000u);
    u32 v = readl_d(AVS_CNT_DIV);
    if (idx == 0)
        v = (div - 1) | (v & 0xFFFF0000u);
    else if (idx == 1)
        v = (v & 0xFFFFu) | ((div - 1) << 16);
    writel_d(AVS_CNT_DIV, v);
}
