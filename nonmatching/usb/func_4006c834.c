/*
 * __USBC_Dev_Rx_ClearStall -- stock 0x4006C834 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR &= ~0x20, then &= ~0x40.
 */
#include "hw/usb.h"

void __USBC_Dev_Rx_ClearStall(void) { REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) & 0xFFDF); REG16(USBC_RXCSR) = (unsigned short)(REG16(USBC_RXCSR) & 0xFFBF); }
