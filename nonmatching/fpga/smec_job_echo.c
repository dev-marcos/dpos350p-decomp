/*
 * smec_job_echo -- stock 0x4004A064 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Job index echoed by the FPGA (selector 0x68).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

u32 smec_job_echo(void)
{
    fpga_selector_command_write(0x68);
    return fpga_data_byte_read();
}
