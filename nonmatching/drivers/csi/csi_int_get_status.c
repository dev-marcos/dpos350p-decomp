/*
 * csi_int_get_status -- stock 0x40065BD4 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel INT_STA (+0x74) bits 0..7 to eight bytes (capture done, frame done, overflows, ...).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_int_get_status(u32 sel, u32 ch, unsigned char *st)
{
    u32 v = csi_readl_t(csi_base_tbl[sel] + (ch << 8) + 0x74);
    int i;
    for (i = 0; i < 8; i++)
        st[i] = (v >> i) & 1;
}
