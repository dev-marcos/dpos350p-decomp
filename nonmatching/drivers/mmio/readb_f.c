/*
 * readb_f -- stock 0x40042E6C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic byte read (SPI-clock source file copy).
 */
#include "hw/drivers.h"
#include "drivers/mmio.h"

u32 readb_f(u32 addr) { return REG8(addr); }
