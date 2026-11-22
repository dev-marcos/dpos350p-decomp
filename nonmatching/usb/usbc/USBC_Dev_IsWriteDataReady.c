/*
 * USBC_Dev_IsWriteDataReady -- stock 0x4006CCB4 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/TX dispatch; 0 otherwise.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_Dev_IsWriteDataReady(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: return __USBC_Dev_ep0_IsWriteDataReady();
    case USBC_EP_TYPE_TX: return __USBC_Dev_Tx_IsWriteDataReady();
    default: return 0;
    }
}
