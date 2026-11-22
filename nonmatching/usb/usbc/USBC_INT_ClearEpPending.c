/*
 * USBC_INT_ClearEpPending -- stock 0x4006C510 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Dispatch on the endpoint type.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_ClearEpPending(u32 ep_type, u32 ep_index)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0:
    case USBC_EP_TYPE_TX: USBC_INT_ClearTxPending(ep_index); break;
    case USBC_EP_TYPE_RX: USBC_INT_ClearRxPending(ep_index); break;
    default: break;
    }
}
