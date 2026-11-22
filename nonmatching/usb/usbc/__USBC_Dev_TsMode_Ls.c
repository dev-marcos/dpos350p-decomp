/*
 * __USBC_Dev_TsMode_Ls -- stock 0x4006C6A4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Same as full speed (b __USBC_Dev_TsMode_Fs).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_TsMode_Ls(void) { __USBC_Dev_TsMode_Fs(); }
