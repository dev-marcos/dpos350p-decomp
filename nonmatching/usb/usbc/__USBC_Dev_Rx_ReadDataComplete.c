/*
 * __USBC_Dev_Rx_ReadDataComplete -- stock 0x4006CBAC (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR &= 0xFFF2.
 */
#include "hw/usb.h"

void __USBC_Dev_Rx_ReadDataComplete(void) { REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) & 0xFFF2); }
