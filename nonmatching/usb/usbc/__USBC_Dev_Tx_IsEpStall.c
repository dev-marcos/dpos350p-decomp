/*
 * __USBC_Dev_Tx_IsEpStall -- stock 0x4006C7E8 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TXCSR bit 5.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_Tx_IsEpStall(void) { return REG16(USBC_CSR0) & 0x20; }
