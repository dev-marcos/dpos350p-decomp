/*
 * __USBC_Dev_ep0_IsReadDataReady -- stock 0x4006CC74 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 bit 0.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_ep0_IsReadDataReady(void) { return REG16(USBC_CSR0) & 1; }
