/*
 * fpga_write_sel_0x62 -- stock 0x4004AA70 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x62 <- SYS_INFO+0x3A.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_write_sel_0x62(void)
{
    fpga_selector_command_write(0x62);
    fpga_data_byte_write(SYS8(0x3A));
}
