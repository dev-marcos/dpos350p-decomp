/*
 * csi_set_size -- stock 0x40065AE8 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel size registers: HSIZE (+0x80) length, VSIZE (+0x84) length, BUF_LEN (+0x88) luma, then chroma.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_set_size(u32 sel, u32 ch, u32 hor_len, u32 ver_len, u32 buf_len_y, u32 buf_len_c)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x80, 0x1FFF0000, hor_len << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x84, 0x1FFF0000, ver_len << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x88, 0x1FFF, buf_len_y);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x88, 0x1FFF0000, buf_len_c << 16);
}
