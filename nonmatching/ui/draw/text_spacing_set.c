/*
 * text_spacing_set -- stock 0x4002DC40 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * *(u16 *)0x42E68BE2 = a.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void text_spacing_set(u32 a)
{
    *(volatile unsigned short *)0x42E68BE2u = a;
}
