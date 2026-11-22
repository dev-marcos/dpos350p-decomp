/*
 * __USBC_Dev_TsMode_Fs -- stock 0x4006C690 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PCTL = PCTL & 0xDF (ISO-update bit 7 / HS-enable bit 5).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_TsMode_Fs(void) { REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) & 0xDF); }
