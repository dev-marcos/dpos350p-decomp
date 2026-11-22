/*
 * USBC_Dev_WriteFAddr -- stock 0x4006C770 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FADDR = r0: tail shared with USBC_Dev_SetAddress_default (name OPEN).
 */
#include "hw/usb.h"

void USBC_Dev_WriteFAddr(u32 v) { REG8(USBC_FADDR) = (unsigned char)v; }
