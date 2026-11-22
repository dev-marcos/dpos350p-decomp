/*
 * USBC_INT_RxPending -- stock 0x4006C468 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTRx.
 */
#include "hw/usb.h"

u32 USBC_INT_RxPending(void) { return REG16(USBC_INTRx); }
