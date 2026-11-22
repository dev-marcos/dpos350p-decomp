/*
 * csi_isr -- stock 0x40065EB0 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSI IRQ (0x74): on frame done set SYS_INFO+0x70E and acknowledge it (0x02).
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

void csi_int_get_status_t(u32 sel, u32 ch, unsigned char *st);
void csi_int_clear_status_t(u32 sel, u32 ch, u32 bits);

void csi_isr(void)
{
    unsigned char st[8];
    csi_int_get_status_t(0, 0, st);
    if (st[1] != 1) return;
    *(unsigned char *)(0x40544E68u + 0x70E) = 1;
    csi_int_clear_status_t(0, 0, 2);
}
