/*
 * __USBC_Dev_ep0_ConfigEp0 -- stock 0x4006CEB0 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 = 0x100 (flush FIFO).
 */
#include "hw/usb.h"

void __USBC_Dev_ep0_ConfigEp0(void) { REG16(USBC_CSR0) = 0x100; }
