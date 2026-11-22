/*
 * avs0_restart -- stock 0x40042DB8 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * AVS counter 0: CNT0 = 0, CTL |= 1.
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"

void avs0_restart(void)
{
    writel_d(AVS_CNT0, 0);
    writel_d(AVS_CNT_CTL, readl_d(AVS_CNT_CTL) | 1);
}
