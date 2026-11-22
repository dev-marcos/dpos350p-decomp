/*
 * USBC_Dev_Ctrl_IsSetupEnd -- stock 0x4006C974 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b __USBC_Dev_ep0_IsSetupEnd.
 */
#include "hw/usb.h"

u32 USBC_Dev_Ctrl_IsSetupEnd(void) { return __USBC_Dev_ep0_IsSetupEnd(); }
