/*
 * __USBC_Dev_Rx_ReadDataHalf -- stock 0x4006CB94 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR &= 0xFFF2.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_Rx_ReadDataHalf(void) { REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) & 0xFFF2); }
