/*
 * USBC_INT_ClearRxPendingAll -- stock 0x4006C488 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTRx = 0xFFFF.
 */
#include "hw/usb.h"

void USBC_INT_ClearRxPendingAll(void) { REG16(USBC_INTRx) = 0xFFFF; }
