/*
 * ccu_set_cpu_pll -- stock 0x400427B0 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU: program PLL_CPU for f Hz (N = f/24 MHz via multiply-high; above 768 MHz P = 1 and N halves), wait for LOCK, AXI/APB dividers before (0x10102) and after (0x20101). DERIVED from the V3s CCU map.
 */
#include "hw/drivers.h"
#include "drivers/ccu.h"
#include "drivers/mmio.h"

#define CCU 0x01C20000u
#define PLL_LOCK 0x10000000u

void ccu_set_cpu_pll(u32 f)
{
    u32 p = 0;
    u32 n = (u32)(((u64)f * 0xB2F4FC08u) >> 32) >> 24;
    if (f >= 0x2DC6C001u) { p = 0x10; n >>= 1; }
    writel_e(CCU + 0x50, 0x00010102u);
    writel_e(CCU + 0x00, p | (((n - 1) << 8) & 0x1F00u) | 0x80000000u);
    while (!(readl_e(CCU + 0x00) & PLL_LOCK))
        ;
    writel_e(CCU + 0x50, 0x00020101u);
}
