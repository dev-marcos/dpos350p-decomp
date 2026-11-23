/*
 * fpga_cmd17_write -- stock 0x40063628 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x275 and +0x274 are set: selector 0x17 and the low byte of the u16 SYS+0x280; else fpga_cmd61_pulse.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_cmd17_write(void)
{
    if (SI(f_275) && SI(f_274)) {
        fpga_selector_command_write(0x17);
        fpga_data_byte_write(SI(f_280) & 0xFF);
    } else {
        fpga_cmd61_pulse();
    }
}
