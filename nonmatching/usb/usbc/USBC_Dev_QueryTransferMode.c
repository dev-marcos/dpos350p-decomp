/*
 * USBC_Dev_QueryTransferMode -- stock 0x4006C730 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PCTL bit 4 (HS flag): 3 = HS, else 2 = FS.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_Dev_QueryTransferMode(void) { return (REG8(USBC_PCTL) & 0x10) ? 3 : 2; }
