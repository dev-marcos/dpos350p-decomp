/*
 * mode_not_0_or_1 -- stock 0x4003BF78 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Returns 0 when v is 0 or 1, else 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 mode_not_0_or_1(u32 v)
{
    return (v == 0 || v == 1) ? 0 : 1;
}
