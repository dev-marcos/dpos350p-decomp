/*
 * fpga_cmd1a_write -- stock 0x4006366C (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x1A, then 1 when SYS+0x275, +0x274 and +0x27A are all set, else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_cmd1a_write(void)
{
    u32 v = 0;
    fpga_selector_command_write(0x1A);
    if (SI(f_275) && SI(f_274) && SI(trigger_mode))
        v = 1;
    fpga_data_byte_write(v);
}
