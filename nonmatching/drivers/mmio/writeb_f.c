/*
 * writeb_f -- stock 0x40042E7C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic byte write.
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"

void writeb_f(u32 addr, u32 v) { REG8(addr) = (unsigned char)v; }
