/*
 * __USBC_Dev_Tx_WriteDataHalf -- stock 0x4006CAA4 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR = (TXCSR & ~4) | 1.
 */
#include "hw/usb.h"

void __USBC_Dev_Tx_WriteDataHalf(void) { REG16(USBC_CSR0) = (unsigned short)((REG16(USBC_CSR0) & 0xFFFB) | 1); }
