/*
 * csi_int_enable -- stock 0x40065BB8 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel INT_EN (+0x70) |= bits.
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

void csi_int_enable(u32 sel, u32 ch, u32 bits) { csi_setbits(csi_base_tbl[sel] + (ch << 8) + 0x70, bits); }
