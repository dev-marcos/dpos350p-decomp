/*
 * __USBC_Dev_ep0_IsEpStall -- stock 0x4006C788 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 bit 2 (sent stall).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 __USBC_Dev_ep0_IsEpStall(void) { return REG16(USBC_CSR0) & 4; }
