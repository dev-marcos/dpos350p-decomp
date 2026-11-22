/*
 * __USBC_Dev_Rx_EnableIsoEp -- stock 0x4006CE24 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR |= 0x4000.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_Rx_EnableIsoEp(void) { REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) | 0x4000); }
