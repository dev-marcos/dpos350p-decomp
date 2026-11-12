/*
 * USBC_INT_ClearMiscPendingAll -- stock 0x4006C570 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTUSB = 0xFF.
 */
#include "hw/usb.h"

void USBC_INT_ClearMiscPendingAll(void) { REG8(USBC_INTUSB) = 0xFF; }
