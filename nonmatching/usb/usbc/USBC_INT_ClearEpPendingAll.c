/*
 * USBC_INT_ClearEpPendingAll -- stock 0x4006C538 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Dispatch on the endpoint type.
 */
#include "hw/usb.h"

void USBC_INT_ClearEpPendingAll(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0:
    case USBC_EP_TYPE_TX: USBC_INT_ClearTxPendingAll(); break;
    case USBC_EP_TYPE_RX: USBC_INT_ClearRxPendingAll(); break;
    default: break;
    }
}
