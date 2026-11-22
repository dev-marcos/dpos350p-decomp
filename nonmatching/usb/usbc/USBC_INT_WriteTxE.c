/*
 * USBC_INT_WriteTxE -- stock 0x4006C4D0 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store r0 to INTTxE: tail shared with USBC_INT_DisableTxAll, no direct callers (name OPEN).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_WriteTxE(u32 v) { REG16(USBC_INTTxE) = (unsigned short)v; }
