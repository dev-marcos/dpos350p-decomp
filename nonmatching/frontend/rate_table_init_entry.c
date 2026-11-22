/*
 * rate_table_init_entry -- stock 0x4004A50C (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to rate_table_init.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "frontend.h"

void rate_table_init_entry(void)
{
    rate_table_init();
}
