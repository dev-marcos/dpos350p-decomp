/*
 * __USBC_Dev_Tx_SendStall -- stock 0x4006C7D4 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR |= 0x10.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_Tx_SendStall(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) | 0x10); }
