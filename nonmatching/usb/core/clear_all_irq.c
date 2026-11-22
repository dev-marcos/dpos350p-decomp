/*
 * clear_all_irq -- stock 0x4006ACB8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clear TX, RX and misc pending interrupts.
 */
#include "hw/usb.h"

void clear_all_irq(void)
{
    USBC_INT_ClearEpPendingAll(USBC_EP_TYPE_TX);
    USBC_INT_ClearEpPendingAll(USBC_EP_TYPE_RX);
    USBC_INT_ClearMiscPendingAll();
}
