/*
 * USBC_Dev_SetAddress -- stock 0x4006C77C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FADDR = address.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_Dev_SetAddress(u32 address) { REG8(USBC_FADDR) = (unsigned char)address; }
