/*
 * half_42e68be2_set -- stock 0x4002DC40 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * *(u16 *)0x42E68BE2 = a.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void half_42e68be2_set(u32 a)
{
    *(volatile unsigned short *)0x42E68BE2u = a;
}
