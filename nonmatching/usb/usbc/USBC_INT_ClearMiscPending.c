/*
 * USBC_INT_ClearMiscPending -- stock 0x4006C564 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTUSB = mask.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_ClearMiscPending(u32 mask) { REG8(USBC_INTUSB) = (unsigned char)mask; }
