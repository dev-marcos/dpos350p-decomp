/*
 * fpga_params_push -- stock 0x40049E30 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sends the acquisition parameters (SYS_INFO 0x1B8, 0x1C0, 0x268, 0x270, 0x26C, 0x08, 0x62C) as jobs 1..7.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_params_push(void)
{
    fpga_param_updater(1, 1, SYS32(0x1B8), SYS8(0x1B8));
    fpga_param_updater(2, 4, SYS32(0x1C0), SYS32(0x1C0));
    fpga_param_updater(3, 4, SYS32(0x268), SYS32(0x268));
    fpga_param_updater(4, 4, SYS32(0x270), SYS32(0x270));
    fpga_param_updater(5, 4, SYS32(0x26C), SYS32(0x26C));
    fpga_param_updater(6, 2, SYS32(0x08), SYS16(0x08));
    fpga_param_updater(7, 4, SYS32(0x62C), SYS32(0x62C));
}
