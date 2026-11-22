/*
 * csi_if_cfg -- stock 0x400658FC (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * IF_CFG (+0x04) from {src_type (bit 21), data_width (9:8), interface (4:0)} as signed bytes.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_if_cfg(u32 sel, const signed char *cfg)
{
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x200000, (u32)(int)cfg[0] << 21);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x1F, (u32)(int)cfg[2]);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x300, (u32)(int)cfg[1] << 8);
}
