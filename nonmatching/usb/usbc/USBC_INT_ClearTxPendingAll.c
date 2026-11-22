/*
 * USBC_INT_ClearTxPendingAll -- stock 0x4006C454 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTTx = 0xFFFF.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_ClearTxPendingAll(void) { REG16(USBC_INTTx) = 0xFFFF; }
