/*
 * __USBC_Dev_TsType_Iso -- stock 0x4006C62C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PCTL = PCTL | 0x80 (ISO-update bit 7 / HS-enable bit 5).
 */
#include "hw/usb.h"

void __USBC_Dev_TsType_Iso(void) { REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) | 0x80); }
