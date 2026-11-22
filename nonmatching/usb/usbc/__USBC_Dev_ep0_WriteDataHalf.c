/*
 * __USBC_Dev_ep0_WriteDataHalf -- stock 0x4006CA84 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 = 0x02 (TxPktRdy).
 */
#include "hw/usb.h"

void __USBC_Dev_ep0_WriteDataHalf(void) { REG16(USBC_CSR0) = 0x02; }
