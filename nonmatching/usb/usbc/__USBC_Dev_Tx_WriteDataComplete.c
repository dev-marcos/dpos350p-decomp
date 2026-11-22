/*
 * __USBC_Dev_Tx_WriteDataComplete -- stock 0x4006CAC0 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Same as the half version.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_Tx_WriteDataComplete(void) { REG16(USBC_CSR0) = (unsigned short)((REG16(USBC_CSR0) & 0xFFFB) | 1); }
