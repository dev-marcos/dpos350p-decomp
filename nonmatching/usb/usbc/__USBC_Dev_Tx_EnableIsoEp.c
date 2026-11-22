/*
 * __USBC_Dev_Tx_EnableIsoEp -- stock 0x4006CD98 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR |= 0x4000.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_Tx_EnableIsoEp(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) | 0x4000); }
