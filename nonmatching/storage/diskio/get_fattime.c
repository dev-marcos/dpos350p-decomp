/*
 * get_fattime -- stock 0x40049DD8 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fixed timestamp 0x53756000 (2021-11-21 12:00:00).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/diskio.h"

u32 get_fattime(void)
{
    return 0x53756000u;
}
