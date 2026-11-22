/*
 * __USBC_Dev_ep0_WriteDataComplete -- stock 0x4006CA94 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 = 0x0A (TxPktRdy | DataEnd).
 */
#include "hw/usb.h"

void __USBC_Dev_ep0_WriteDataComplete(void) { REG16(USBC_CSR0) = 0x0A; }
