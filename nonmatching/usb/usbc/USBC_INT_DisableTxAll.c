/*
 * USBC_INT_DisableTxAll -- stock 0x4006C4CC (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTTxE = 0 (mov r0,#0, then falls into the store at 0x4006C4D0).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_DisableTxAll(void) { REG16(USBC_INTTxE) = 0; }
