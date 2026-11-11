/*
 * readl_c -- stock 0x40068E64 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic 32-bit register read (one copy per source file in the stock image).
 */
#include "hw/v3s.h"

u32 readl_c(u32 addr)
{
    return REG32(addr);
}
