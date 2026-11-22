/*
 * word_42e68c74_set -- stock 0x4002CCCC (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * *(u32 *)0x42E68C74 = a.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void word_42e68c74_set(u32 a)
{
    *(volatile u32 *)0x42E68C74u = a;
}
