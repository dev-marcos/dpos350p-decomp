/*
 * USBC_INT_TxPending -- stock 0x4006C434 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTTx.
 */
#include "hw/usb.h"

u32 USBC_INT_TxPending(void) { return REG16(USBC_INTTx); }
