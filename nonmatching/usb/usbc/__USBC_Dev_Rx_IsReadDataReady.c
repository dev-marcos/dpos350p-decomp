/*
 * __USBC_Dev_Rx_IsReadDataReady -- stock 0x4006CC64 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RXCSR bit 0.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_Rx_IsReadDataReady(void) { return REG16(USBC_RXCSR) & 1; }
