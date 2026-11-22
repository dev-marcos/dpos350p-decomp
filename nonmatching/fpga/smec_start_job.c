/*
 * smec_start_job -- stock 0x40049FDC (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Start (0x68 = 1), wait for selector 0x64 bit 0, release (0x68 = 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void smec_start_job(void)
{
    fpga_selector_command_write(0x68);
    fpga_data_byte_write(1);
    do {
        fpga_selector_command_write(0x64);
    } while (!(fpga_data_byte_read() & 1));
    fpga_selector_command_write(0x68);
    fpga_data_byte_write(0);
}
