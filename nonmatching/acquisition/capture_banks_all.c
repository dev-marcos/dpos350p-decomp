/*
 * capture_banks_all -- stock 0x400406DC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * capture_bank(0..7).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void capture_banks_all(void)
{
    u32 i;
    for (i = 0; i < 8; i++)
        capture_bank(i & 0xFF);
}
