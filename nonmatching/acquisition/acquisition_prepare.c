/*
 * acquisition_prepare -- stock 0x4003FA10 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Acquisition start sequence: 0x0F=0, 0x01: 1 then 0, 0x11: 0 then 1, 0x02 <- SYS_INFO+0xBB3, 0x0F=1. (name: Hack STOCK_LIKE_MINIMAL_RUNTIME.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/fpga_bus.h"
#include "sys_info.h"

void acquisition_prepare(void)
{
    fpga_selector_command_write(0x0F); fpga_data_byte_write(0);
    fpga_selector_command_write(0x01); fpga_data_byte_write(1);
    fpga_selector_command_write(0x01); fpga_data_byte_write(0);
    fpga_selector_command_write(0x11); fpga_data_byte_write(0);
    fpga_selector_command_write(0x11); fpga_data_byte_write(1);
    fpga_selector_command_write(0x02); fpga_data_byte_write(SI(f_bb3));
    fpga_selector_command_write(0x0F); fpga_data_byte_write(1);
}
