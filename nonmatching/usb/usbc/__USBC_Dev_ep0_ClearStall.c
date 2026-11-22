/*
 * __USBC_Dev_ep0_ClearStall -- stock 0x4006C7AC (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR0 &= ~0x20, then &= ~0x04.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_ep0_ClearStall(void) { REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) & 0xFFDF); REG16(USBC_CSR0) = (unsigned short)(REG16(USBC_CSR0) & 0xFFFB); }
