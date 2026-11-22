/*
 * USBC_INT_ClearRxPending -- stock 0x4006C474 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTRx = 1 << ep.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_ClearRxPending(u32 ep_index) { REG16(USBC_INTRx) = (unsigned short)(1u << ep_index); }
