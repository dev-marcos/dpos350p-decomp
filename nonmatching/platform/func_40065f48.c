/*
 * csi_clk_init -- stock 0x40065F48 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSI bus gate/reset, CCU 0x01C20100 bit 1, PLL at 0x01C2002C enable + wait LOCK, CSI clock 0x01C20134 = 0x81008000.
 */
#include "hw/platform.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)
u32 csi_readl(u32 addr);
void csi_writel(u32 addr, u32 v);
void csi_setbits(u32 addr, u32 bits);
void csi_clrbits(u32 addr, u32 bits);
void csi_clrsetbits(u32 addr, u32 clr, u32 set);
void dcache_clean_inv_all(void);
void csi_writel_t(u32 addr, u32 v);
u32 csi_readl_t(u32 addr);

void ccu_gate_on(s32 code);

void csi_clk_init(void)
{
    ccu_gate_on(0x0C885888);
    csi_writel(0x01C20100u, csi_readl(0x01C20100u) | 2);
    csi_writel(0x01C2002Cu, csi_readl(0x01C2002Cu) | 0x80000000u);
    while (!(csi_readl(0x01C2002Cu) & 0x10000000u))
        ;
    csi_writel(0x01C20134u, 0x81008000u);
}
