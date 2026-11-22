/*
 * usb_phy_nop -- stock 0x4006C108 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Empty function (bx lr); no EasyUSB counterpart identified (name OPEN).
 */
#include "hw/usb.h"
#include "usb/phy.h"

void usb_phy_nop(void) { }
