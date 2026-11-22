/*
 * __USBC_Dev_ep0_ReadDataHalf -- stock 0x4006CB74 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 = 0x40 (ServicedRxPktRdy).
 */
#include "hw/usb.h"

void __USBC_Dev_ep0_ReadDataHalf(void) { REG16(USBC_CSR0) = 0x40; }
