/*
 * __USBC_Dev_ep0_ReadDataComplete -- stock 0x4006CB84 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 = 0x48 (ServicedRxPktRdy | DataEnd).
 */
#include "hw/usb.h"

void __USBC_Dev_ep0_ReadDataComplete(void) { REG16(USBC_CSR0) = 0x48; }
