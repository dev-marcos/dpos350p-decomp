/*
 * csi_set_buffer_address -- stock 0x40065A60 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * BUFn of channel ch (+0x50 + ch*0x100 + buf*4) = 64-bit address / 4.
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

void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr)
{
    csi_clrsetbits(csi_base_tbl[sel] + (ch << 8) + (buf << 2) + 0x50, 0xFFFFFFFFu, (u32)(addr >> 2));
}
