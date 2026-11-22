/*
 * csi_if_cfg -- stock 0x400658FC (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * IF_CFG (+0x04) from {src_type (bit 21), data_width (9:8), interface (4:0)} as signed bytes.
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

void csi_if_cfg(u32 sel, const signed char *cfg)
{
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x200000, (u32)(int)cfg[0] << 21);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x1F, (u32)(int)cfg[2]);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x300, (u32)(int)cfg[1] << 8);
}
