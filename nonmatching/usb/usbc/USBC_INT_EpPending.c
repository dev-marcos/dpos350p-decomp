/*
 * USBC_INT_EpPending -- stock 0x4006C4EC (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/TX -> INTTx, RX -> INTRx, else 0.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_INT_EpPending(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0:
    case USBC_EP_TYPE_TX: return USBC_INT_TxPending();
    case USBC_EP_TYPE_RX: return USBC_INT_RxPending();
    default: return 0;
    }
}
