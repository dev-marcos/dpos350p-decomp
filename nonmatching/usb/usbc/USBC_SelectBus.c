/*
 * USBC_SelectBus -- stock 0x4006C264 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * VEND0: select DMA (DRQ for a TX/RX endpoint) or PIO.
 */
#include "hw/usb.h"

void USBC_SelectBus(u32 io_type, u32 ep_type, u32 ep_index)
{
    u32 v = REG8(USBC_VEND0);
    if (io_type == 1) {
        if (ep_type == USBC_EP_TYPE_TX)
            v |= ((ep_index - 1) << 1) << 1 | 1;
        else
            v |= ((ep_index << 1) - 1) << 1 | 1;
    } else {
        v = 0;
    }
    REG8(USBC_VEND0) = (unsigned char)v;
}
