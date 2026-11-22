/*
 * readl_f -- stock 0x40042E74 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic register read.
 */
#include "hw/drivers.h"

u32 readl_f(u32 addr) { return REG32(addr); }
