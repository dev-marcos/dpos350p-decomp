/*
 * __USBC_Dev_Tx_EnableBulkEp -- stock 0x4006CDC4 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR &= ~0x4000.
 */
#include "hw/usb.h"

void __USBC_Dev_Tx_EnableBulkEp(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) & 0xBFFF); }
