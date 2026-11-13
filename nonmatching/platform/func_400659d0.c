/*
 * csi_fmt_cfg -- stock 0x400659D0 (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel CH_CFG (+0x44 + ch*0x100): input format (23:20), output format (19:16), field select (11:10), input seq (9:8).
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

void csi_fmt_cfg(u32 sel, u32 ch, const signed char *fmt)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xF00000, (u32)(int)fmt[0] << 20);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xF0000, (u32)(int)fmt[1] << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xC00, (u32)(int)fmt[2] << 10);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0x300, (u32)(int)fmt[3] << 8);
}
