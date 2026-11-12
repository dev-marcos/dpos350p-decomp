/*
 * USBC_Clear_Feature_HALT -- stock 0x4006CF24 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TX: TXCSR = 0x2048; RX: RXCSR = 0x90.
 */
#include "hw/usb.h"

void USBC_Clear_Feature_HALT(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: REG16(USBC_CSR0) = 0x2048; break;
    case USBC_EP_TYPE_RX: REG16(USBC_RXCSR) = 0x90; break;
    default: break;
    }
}
