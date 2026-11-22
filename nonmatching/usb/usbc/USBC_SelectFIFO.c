/*
 * USBC_SelectFIFO -- stock 0x4006C998 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Endpoint FIFO address 0x01C19000 + 4*ep.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void *USBC_SelectFIFO(u32 ep_index) { return (void *)USBC_EPFIFO(ep_index); }
