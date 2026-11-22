/*
 * USBC_INT_EnableEp -- stock 0x4006C580 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TX/RX endpoint interrupt enable.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_EnableEp(u32 ep_type, u32 ep_index)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: USBC_INT_EnableTxEp(ep_index); break;
    case USBC_EP_TYPE_RX: USBC_INT_EnableRxEp(ep_index); break;
    default: break;
    }
}
