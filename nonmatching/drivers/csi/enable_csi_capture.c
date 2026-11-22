/*
 * enable_csi_capture -- stock 0x400660AC (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Start capture on all configured channels in mode 1 (single).
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

void csi_capture_start_t(u32 sel, u32 ch_total, u32 mode);

void enable_csi_capture(void) { csi_capture_start_t(0, *(u32 *)(0x42E5549Cu + 0x10), 1); }
