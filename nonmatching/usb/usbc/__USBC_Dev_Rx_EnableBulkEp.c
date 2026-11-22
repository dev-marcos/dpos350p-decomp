/*
 * __USBC_Dev_Rx_EnableBulkEp -- stock 0x4006CE50 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR &= ~0x4000.
 */
#include "hw/usb.h"

void __USBC_Dev_Rx_EnableBulkEp(void) { REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) & 0xBFFF); }
