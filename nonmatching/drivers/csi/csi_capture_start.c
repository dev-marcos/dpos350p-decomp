/*
 * csi_capture_start -- stock 0x40065A98 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CAP (+0x08) = mode in the byte of the channel count (1..4), 0 otherwise.
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

void csi_capture_start(u32 sel, u32 ch_total, u32 mode)
{
    u32 v;
    if (ch_total == 4) v = mode << 24;
    else if (ch_total == 3) v = mode << 16;
    else if (ch_total == 2) v = mode << 8;
    else if (ch_total == 1) v = mode;
    else v = 0;
    csi_writel_t(csi_base_tbl[sel] + 8, v);
}
