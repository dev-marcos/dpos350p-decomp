/*
 * __USBC_Dev_ep0_SendStall -- stock 0x4006C798 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 |= 0x20.
 */
#include "hw/usb.h"

void __USBC_Dev_ep0_SendStall(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) | 0x20); }
