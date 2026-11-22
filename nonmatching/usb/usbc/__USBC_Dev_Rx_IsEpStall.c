/*
 * __USBC_Dev_Rx_IsEpStall -- stock 0x4006C824 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR bit 6.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_Rx_IsEpStall(void) { return REG16(USBC_RXCSR) & 0x40; }
