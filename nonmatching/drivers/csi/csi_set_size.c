/*
 * csi_set_size -- stock 0x40065AE8 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel size registers: HSIZE (+0x80) length, VSIZE (+0x84) length, BUF_LEN (+0x88) luma, then chroma.
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

void csi_set_size(u32 sel, u32 ch, u32 hor_len, u32 ver_len, u32 buf_len_y, u32 buf_len_c)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x80, 0x1FFF0000, hor_len << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x84, 0x1FFF0000, ver_len << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x88, 0x1FFF, buf_len_y);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x88, 0x1FFF0000, buf_len_c << 16);
}
