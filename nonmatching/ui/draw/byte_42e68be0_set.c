/*
 * byte_42e68be0_set -- stock 0x4002DC74 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * *(u8 *)0x42E68BE0 = a.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void byte_42e68be0_set(u32 a)
{
    *(volatile unsigned char *)0x42E68BE0u = a;
}
