/*
 * USBC_Dev_Ctrl_ClearSetupEnd -- stock 0x4006C978 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 |= 0x80 (serviced setup end).
 */
#include "hw/usb.h"

void USBC_Dev_Ctrl_ClearSetupEnd(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) | 0x80); }
