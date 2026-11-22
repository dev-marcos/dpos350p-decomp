/*
 * csi_set_offset -- stock 0x40065B70 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel HSIZE/VSIZE start (bits 12:0).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_set_offset(u32 sel, u32 ch, u32 hor_start, u32 ver_start)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x80, 0x1FFF, hor_start);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x84, 0x1FFF, ver_start);
}
