/*
 * USBC_INT_MiscPending -- stock 0x4006C558 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTUSB.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_INT_MiscPending(void) { return REG8(USBC_INTUSB); }
