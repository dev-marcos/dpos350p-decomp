/*
 * __USBC_Dev_Rx_SendStall -- stock 0x4006C810 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR |= 0x20.
 */
#include "hw/usb.h"

void __USBC_Dev_Rx_SendStall(void) { REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) | 0x20); }
