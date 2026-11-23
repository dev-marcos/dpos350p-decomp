/*
 * fpga_params_push -- stock 0x40049E30 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sends the acquisition parameters (SYS_INFO 0x1B8, 0x1C0, 0x268, 0x270, 0x26C, 0x08, 0x62C) as jobs 1..7.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_params_push(void)
{
    fpga_param_updater(1, 1, SYS32(0x1B8), SI(f_1b8));
    fpga_param_updater(2, 4, SI(acq_geometry_1c0), SI(acq_geometry_1c0));
    fpga_param_updater(3, 4, SI(acq_geometry_268), SI(acq_geometry_268));
    fpga_param_updater(4, 4, SI(acq_geometry_270), SI(acq_geometry_270));
    fpga_param_updater(5, 4, SI(acq_geometry_26c), SI(acq_geometry_26c));
    fpga_param_updater(6, 2, SYS32(0x08), SI(plot_width));
    fpga_param_updater(7, 4, SI(acq_geometry_62c), SI(acq_geometry_62c));
}
