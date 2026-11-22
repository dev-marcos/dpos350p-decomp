/*
 * USBC_GetActiveEp -- stock 0x4006C5C0 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EPIND.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_GetActiveEp(void) { return REG8(USBC_EPIND); }
