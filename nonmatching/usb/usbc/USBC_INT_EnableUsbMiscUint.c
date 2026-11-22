/*
 * USBC_INT_EnableUsbMiscUint -- stock 0x4006C5AC (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTUSBE |= mask.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_EnableUsbMiscUint(u32 mask) { REG8(USBC_INTUSBE) = (unsigned char)(mask | REG8(USBC_INTUSBE)); }
