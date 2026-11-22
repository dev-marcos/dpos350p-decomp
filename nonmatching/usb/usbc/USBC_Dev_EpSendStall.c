/*
 * USBC_Dev_EpSendStall -- stock 0x4006C8B4 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/TX/RX dispatch; -1 for another type.
 */
#include "hw/usb.h"

int USBC_Dev_EpSendStall(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_SendStall(); break;
    case USBC_EP_TYPE_TX: __USBC_Dev_Tx_SendStall(); break;
    case USBC_EP_TYPE_RX: __USBC_Dev_Rx_SendStall(); break;
    default: return -1;
    }
    return 0;
}
