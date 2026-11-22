/*
 * USBC_INT_DisableRxAll -- stock 0x4006C4DC (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INTRxE = 0 (falls into 0x4006C4E0).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_INT_DisableRxAll(void) { REG16(USBC_INTRxE) = 0; }
