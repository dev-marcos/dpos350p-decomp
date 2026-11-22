/*
 * USBC_Dev_Read_EP0_CSR -- stock 0x4006C98C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_Dev_Read_EP0_CSR(void) { return REG16(USBC_CSR0); }
