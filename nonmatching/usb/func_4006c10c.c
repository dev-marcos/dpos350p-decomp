/*
 * USBC_WakeUp_ClearChangeDetect -- stock 0x4006C10C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clear the write-1-to-clear VBUS/ID/DPDM change bits (6:4) from a value.
 */
#include "hw/usb.h"

u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val) { return reg_val & ~0x70u; }
