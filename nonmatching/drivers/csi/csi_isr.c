/*
 * csi_isr -- stock 0x40065EB0 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSI IRQ (0x74): on frame done set SYS_INFO+0x70E and acknowledge it (0x02).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_isr(void)
{
    unsigned char st[8];
    csi_int_get_status_t(0, 0, st);
    if (st[1] != 1) return;
    *(unsigned char *)(0x40544E68u + 0x70E) = 1;
    csi_int_clear_status_t(0, 0, 2);
}
