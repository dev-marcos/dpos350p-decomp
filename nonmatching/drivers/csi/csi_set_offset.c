/*
 * csi_set_offset -- stock 0x40065B70 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel HSIZE/VSIZE start (bits 12:0).
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

void csi_set_offset(u32 sel, u32 ch, u32 hor_start, u32 ver_start)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x80, 0x1FFF, hor_start);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x84, 0x1FFF, ver_start);
}
