/*
 * USBC_INT_DisableEpAll -- stock 0x4006C748 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TX/RX: all endpoint interrupts off.
 */
#include "hw/usb.h"

void USBC_INT_DisableEpAll(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: USBC_INT_DisableTxAll(); break;
    case USBC_EP_TYPE_RX: USBC_INT_DisableRxAll(); break;
    default: break;
    }
}
