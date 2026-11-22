/*
 * __USBC_Dev_ep0_IsSetupEnd -- stock 0x4006C964 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 bit 4.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_ep0_IsSetupEnd(void) { return REG16(USBC_CSR0) & 0x10; }
