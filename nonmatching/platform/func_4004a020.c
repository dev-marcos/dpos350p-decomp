/*
 * smec_job_valid -- stock 0x4004A020 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Result-valid flag (selector 0x67 bit 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 smec_job_valid(void)
{
    fpga_selector_command_write(0x67);
    return fpga_data_byte_read() & 1;
}
