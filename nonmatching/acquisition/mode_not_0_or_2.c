/*
 * mode_not_0_or_2 -- stock 0x4003BF58 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Returns 0 when v is 0 or 2, else 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

u32 mode_not_0_or_2(u32 v)
{
    return (v == 0 || v == 2) ? 0 : 1;
}
