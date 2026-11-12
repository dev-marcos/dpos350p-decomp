/*
 * __USBC_Dev_Tx_ClearStall -- stock 0x4006C7F8 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR &= ~0x30.
 */
#include "hw/usb.h"

void __USBC_Dev_Tx_ClearStall(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) & 0xFFCF); }
