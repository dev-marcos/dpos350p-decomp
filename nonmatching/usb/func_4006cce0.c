/*
 * USBC_ReadLenFromFifo -- stock 0x4006CCE0 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * COUNT0 (EP0) or RXCOUNT (RX), same register; 0 otherwise.
 */
#include "hw/usb.h"

u32 USBC_ReadLenFromFifo(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: return REG16(USBC_COUNT0);
    case USBC_EP_TYPE_RX: return REG16(USBC_COUNT0);
    default: return 0;
    }
}
