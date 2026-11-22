/*
 * USBC_Dev_EpClearStall -- stock 0x4006C90C (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/TX/RX dispatch; -1 for another type.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

int USBC_Dev_EpClearStall(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_ClearStall(); break;
    case USBC_EP_TYPE_TX: __USBC_Dev_Tx_ClearStall(); break;
    case USBC_EP_TYPE_RX: __USBC_Dev_Rx_ClearStall(); break;
    default: return -1;
    }
    return 0;
}
