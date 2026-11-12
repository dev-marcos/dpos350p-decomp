/*
 * USBC_INT_DisableUsbMiscAll -- stock 0x4006C424 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTUSBE = 0.
 */
#include "hw/usb.h"

void USBC_INT_DisableUsbMiscAll(void) { REG8(USBC_INTUSBE) = 0; }
