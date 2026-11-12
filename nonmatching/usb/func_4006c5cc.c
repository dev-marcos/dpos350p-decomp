/*
 * USBC_SelectActiveEp -- stock 0x4006C5CC (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EPIND = ep.
 */
#include "hw/usb.h"

void USBC_SelectActiveEp(u32 ep_index) { REG8(USBC_EPIND) = (unsigned char)ep_index; }
