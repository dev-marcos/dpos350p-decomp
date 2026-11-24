/*
 * csi_set_base_addr -- stock 0x400658BC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store the register base of CSI sel (0 or 1); -1 otherwise.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

int csi_set_base_addr(u32 sel, u32 addr)
{
    if (sel >= 2) return -1;
    csi_base_tbl[sel] = addr;
    return 0;
}
