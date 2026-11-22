/*
 * csi_readl_t -- stock 0x40065890 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_readl.
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

u32 csi_readl_t(u32 addr) { return csi_readl(addr); }
