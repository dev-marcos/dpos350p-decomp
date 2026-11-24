/*
 * csi_set_buffer_address -- stock 0x40065A60 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * BUFn of channel ch (+0x50 + ch*0x100 + buf*4) = 64-bit address / 4.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr)
{
    csi_clrsetbits(csi_base_tbl[sel] + (ch << 8) + (buf << 2) + 0x50, 0xFFFFFFFFu, (u32)(addr >> 2));
}
