/*
 * __USBC_Dev_ReadDataComplete -- stock 0x4006CC00 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/RX dispatch; -1 otherwise.
 */
#include "hw/usb.h"

int __USBC_Dev_ReadDataComplete(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_ReadDataComplete(); break;
    case USBC_EP_TYPE_RX: __USBC_Dev_Rx_ReadDataComplete(); break;
    default: return -1;
    }
    return 0;
}
