/*
 * USBC_Dev_IsReadDataReady -- stock 0x4006CC94 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/RX dispatch; 0 otherwise.
 */
#include "hw/usb.h"

u32 USBC_Dev_IsReadDataReady(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: return __USBC_Dev_ep0_IsReadDataReady();
    case USBC_EP_TYPE_RX: return __USBC_Dev_Rx_IsReadDataReady();
    default: return 0;
    }
}
