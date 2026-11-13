/*
 * csi_init_cfg -- stock 0x40065C80 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel 0 format {0, 0, 2, ?}, interface {0, 4, 0}, timing {0, cfg[2], cfg[1], cfg[3]}; returns 0. The fourth format byte is uninitialised stack in the stock code (0 under the test harness; OPEN).
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

void csi_fmt_cfg(u32 sel, u32 ch, const signed char *fmt);
void csi_if_cfg(u32 sel, const signed char *cfg);
void csi_timing_cfg(u32 sel, const signed char *cfg);

int csi_init_cfg(u32 sel, const signed char *cfg)
{
    signed char fmt[4] = {0, 0, 2, 0}, ifc[4] = {0, 4, 0, 0}, tim[4];
    csi_fmt_cfg(sel, 0, fmt);
    csi_if_cfg(sel, ifc);
    tim[0] = 0; tim[1] = cfg[2]; tim[2] = cfg[1]; tim[3] = cfg[3];
    csi_timing_cfg(sel, tim);
    return 0;
}
