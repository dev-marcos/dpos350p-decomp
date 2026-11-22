/*
 * __USBC_Dev_WriteDataComplete -- stock 0x4006CB28 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/TX dispatch; -1 otherwise.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

int __USBC_Dev_WriteDataComplete(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_WriteDataComplete(); break;
    case USBC_EP_TYPE_TX: __USBC_Dev_Tx_WriteDataComplete(); break;
    default: return -1;
    }
    return 0;
}
