/*
 * csi_set_frame_flag -- stock 0x4006610C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS_INFO+0x70E = v, then flush the D-cache.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_set_frame_flag(u32 v)
{
    *(unsigned char *)(0x40544E68u + 0x70E) = (unsigned char)v;
    dcache_clean_inv_all();
}
