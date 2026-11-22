/*
 * USBC_INT_WriteRxE -- stock 0x4006C4E0 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store r0 to INTRxE: tail shared with USBC_INT_DisableRxAll (name OPEN).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_WriteRxE(u32 v) { REG16(USBC_INTRxE) = (unsigned short)v; }
