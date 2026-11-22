/*
 * fpga_write_sel_0x2f -- stock 0x4004AB60 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x2F <- SYS_INFO+0x3F.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_write_sel_0x2f(void)
{
    fpga_selector_command_write(0x2F);
    fpga_data_byte_write(SYS8(0x3F));
}
