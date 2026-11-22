/*
 * USBC_ConfigFIFO_Base -- stock 0x4006C368 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SRAM controller +0x04: USB FIFO 8 KiB mode.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_ConfigFIFO_Base(void)
{
    u32 v = REG32(0x01C00004u);
    v = (v & ~3u) | 1;
    REG32(0x01C00004u) = v;
}
