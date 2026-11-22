/*
 * csi_reset -- stock 0x40065C68 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Disable then enable.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_reset(u32 sel)
{
    csi_disable(sel);
    csi_enable(sel);
}
