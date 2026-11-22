/*
 * USBC_INT_ClearTxPending -- stock 0x4006C440 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTTx = 1 << ep.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_ClearTxPending(u32 ep_index) { REG16(USBC_INTTx) = (unsigned short)(1u << ep_index); }
