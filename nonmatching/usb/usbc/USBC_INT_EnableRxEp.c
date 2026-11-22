/*
 * USBC_INT_EnableRxEp -- stock 0x4006C4B4 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTRxE |= 1 << ep.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_EnableRxEp(u32 ep_index) { REG16(USBC_INTRxE) = (unsigned short)(REG16(USBC_INTRxE) | (1u << ep_index)); }
