/*
 * selector_0x6a_one_byte_job_index -- stock 0x40049F60 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Job index (selector 0x6A). (name: Hack SMEC.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void selector_0x6a_one_byte_job_index(u32 v)
{
    fpga_selector_command_write(0x6A);
    fpga_data_byte_write(v);
}
