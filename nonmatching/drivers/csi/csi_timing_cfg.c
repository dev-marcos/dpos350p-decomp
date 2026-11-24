/*
 * csi_timing_cfg -- stock 0x40065954 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * IF_CFG polarity bits: vref (18), href (17), clock edge (16, set when cfg[3] == 1), field (19).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_timing_cfg(u32 sel, const signed char *cfg)
{
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x40000, (u32)(int)cfg[1] << 18);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x20000, (u32)(int)cfg[2] << 17);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x10000, cfg[3] == 1 ? 0x10000 : 0);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x80000, (u32)(int)cfg[0] << 19);
}
