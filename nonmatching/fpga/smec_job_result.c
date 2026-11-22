/*
 * smec_job_result -- stock 0x4004A080 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 32-bit job result, MSB first (selector 0x66). The stock body continues at 0x4004A094 (split by the analyser).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

u32 smec_job_result(void)
{
    u32 v;
    fpga_selector_command_write(0x66);
    v = fpga_data_byte_read();
    v = fpga_data_byte_read() | (v << 8);
    v = fpga_data_byte_read() | (v << 8);
    return fpga_data_byte_read() | (v << 8);
}
