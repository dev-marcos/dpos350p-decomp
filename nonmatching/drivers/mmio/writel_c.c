/*
 * writel_c -- stock 0x40068E6C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generic 32-bit register write (one copy per source file in the stock image).
 */
#include "hw/v3s.h"
#include "drivers/mmio.h"

void writel_c(u32 addr, u32 v)
{
    REG32(addr) = v;
}
