/*
 * __USBC_Dev_Tx_IsWriteDataReady -- stock 0x4006CC54 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR bit 0.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_Tx_IsWriteDataReady(void) { return REG16(USBC_CSR0) & 1; }
