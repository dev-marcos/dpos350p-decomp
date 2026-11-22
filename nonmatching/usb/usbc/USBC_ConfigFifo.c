/*
 * USBC_ConfigFifo -- stock 0x4006CA50 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TX/RX dispatch (EP0 not supported).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_ConfigFifo(u32 ep_type, u32 is_double_fifo, u32 fifo_size, u32 fifo_addr)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: __USBC_ConfigFifo_TxEp(is_double_fifo, fifo_size, fifo_addr); break;
    case USBC_EP_TYPE_RX: __USBC_ConfigFifo_RxEp(is_double_fifo, fifo_size, fifo_addr); break;
    default: break;
    }
}
