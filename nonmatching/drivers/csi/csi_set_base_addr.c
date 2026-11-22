/*
 * csi_set_base_addr -- stock 0x400658BC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store the register base of CSI sel (0 or 1); -1 otherwise.
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

int csi_set_base_addr(u32 sel, u32 addr)
{
    if (sel >= 2) return -1;
    csi_base_tbl[sel] = addr;
    return 0;
}
