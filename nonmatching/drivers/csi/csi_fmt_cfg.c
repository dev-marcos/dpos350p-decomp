/*
 * csi_fmt_cfg -- stock 0x400659D0 (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel CH_CFG (+0x44 + ch*0x100): input format (23:20), output format (19:16), field select (11:10), input seq (9:8).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_fmt_cfg(u32 sel, u32 ch, const signed char *fmt)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xF00000, (u32)(int)fmt[0] << 20);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xF0000, (u32)(int)fmt[1] << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xC00, (u32)(int)fmt[2] << 10);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0x300, (u32)(int)fmt[3] << 8);
}
