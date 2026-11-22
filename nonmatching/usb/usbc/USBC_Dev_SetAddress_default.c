/*
 * USBC_Dev_SetAddress_default -- stock 0x4006C76C (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FADDR = 0 (falls into 0x4006C770).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_Dev_SetAddress_default(void) { REG8(USBC_FADDR) = 0; }
