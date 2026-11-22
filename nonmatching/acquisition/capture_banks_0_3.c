/*
 * capture_banks_0_3 -- stock 0x400406FC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * capture_bank(0..3).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void capture_banks_0_3(void)
{
    u32 i;
    for (i = 0; i < 4; i++)
        capture_bank(i & 0xFF);
}
