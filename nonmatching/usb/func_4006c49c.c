/*
 * USBC_INT_EnableTxEp -- stock 0x4006C49C (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTTxE |= 1 << ep.
 */
#include "hw/usb.h"

void USBC_INT_EnableTxEp(u32 ep_index) { REG16(USBC_INTTxE) = (unsigned short)(REG16(USBC_INTTxE) | (1u << ep_index)); }
