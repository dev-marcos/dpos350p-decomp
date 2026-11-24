/*
 * csi_clk_init -- stock 0x40065F48 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSI bus gate/reset, CCU 0x01C20100 bit 1, PLL at 0x01C2002C enable + wait LOCK, CSI clock 0x01C20134 = 0x81008000.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_clk_init(void)
{
    ccu_gate_on(0x0C885888);
    csi_writel(0x01C20100u, csi_readl(0x01C20100u) | 2);
    csi_writel(0x01C2002Cu, csi_readl(0x01C2002Cu) | 0x80000000u);
    while (!(csi_readl(0x01C2002Cu) & 0x10000000u))
        ;
    csi_writel(0x01C20134u, 0x81008000u);
}
