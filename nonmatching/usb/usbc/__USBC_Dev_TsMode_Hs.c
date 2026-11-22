/*
 * __USBC_Dev_TsMode_Hs -- stock 0x4006C67C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PCTL = PCTL | 0x20 (ISO-update bit 7 / HS-enable bit 5).
 */
#include "hw/usb.h"

void __USBC_Dev_TsMode_Hs(void) { REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) | 0x20); }
