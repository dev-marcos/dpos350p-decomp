/*
 * capture_banks_4_7 -- stock 0x4004071C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * capture_bank(4..7).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void capture_bank(u32 i);

void capture_banks_4_7(void)
{
    u32 i;
    for (i = 4; i < 8; i++)
        capture_bank(i & 0xFF);
}
