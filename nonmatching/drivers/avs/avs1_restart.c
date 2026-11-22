/*
 * avs1_restart -- stock 0x40042DE4 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * AVS counter 1: CNT1 = 0, CTL |= 2.
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"

void avs1_restart(void)
{
    writel_d(AVS_CNT1, 0);
    writel_d(AVS_CNT_CTL, readl_d(AVS_CNT_CTL) | 2);
}
