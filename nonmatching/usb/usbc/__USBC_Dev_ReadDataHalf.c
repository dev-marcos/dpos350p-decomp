/*
 * __USBC_Dev_ReadDataHalf -- stock 0x4006CBC4 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/RX dispatch; -1 otherwise.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

int __USBC_Dev_ReadDataHalf(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_ReadDataHalf(); break;
    case USBC_EP_TYPE_RX: __USBC_Dev_Rx_ReadDataHalf(); break;
    default: return -1;
    }
    return 0;
}
