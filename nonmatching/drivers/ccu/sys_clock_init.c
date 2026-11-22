/*
 * sys_clock_init -- stock 0x4004282C (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU: CPU clock (argument < 1500 is taken as MHz, rounded down to a multiple of 24 MHz), PLL_VIDEO = 0x91004107, PLL_PERIPH0 = 0x90041811 (wait LOCK), AHB/APB0 = 0x3180, MBUS = 0x81000003. Hack: sys_clock_init(1152000000).
 */
#include "hw/drivers.h"
#include "drivers/ccu.h"
#include "drivers/mmio.h"

#define CCU 0x01C20000u
#define PLL_LOCK 0x10000000u

void sys_clock_init(s32 f)
{
    if (f < 1500) f = f * 1000000;
    s32 q = (s32)(((s64)f * 0x165E9F81) >> 32) >> 21;
    q += (u32)q >> 31;                         /* signed division by 24 000 000 */
    ccu_set_cpu_pll((u32)(q * 24000000));
    writel_e(CCU + 0x10, 0x91004107u);
    writel_e(CCU + 0x28, 0x90041811u);
    while (!(readl_e(CCU + 0x28) & PLL_LOCK))
        ;
    writel_e(CCU + 0x28 + 0x2C, 0x3180);
    writel_e(CCU + 0x28 + 0x134, 0x81000003u);
}
