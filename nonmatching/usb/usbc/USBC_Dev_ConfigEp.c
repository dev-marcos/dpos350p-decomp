/*
 * USBC_Dev_ConfigEp -- stock 0x4006CEC0 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0/TX/RX dispatch; -1 otherwise.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

int USBC_Dev_ConfigEp(u32 ts_type, u32 ep_type, u32 is_double_fifo, u32 ep_MaxPkt)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_ConfigEp0(); break;
    case USBC_EP_TYPE_TX: __USBC_Dev_Tx_ConfigEp(ts_type, is_double_fifo, ep_MaxPkt); break;
    case USBC_EP_TYPE_RX: __USBC_Dev_Rx_ConfigEp(ts_type, is_double_fifo, ep_MaxPkt); break;
    default: return -1;
    }
    return 0;
}
