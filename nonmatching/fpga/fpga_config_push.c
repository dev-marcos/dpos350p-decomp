/*
 * fpga_config_push -- stock 0x40063328 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * fpga_cmd61_write, fpga_cmd0c_write, fpga_cmd1a_write, fpga_cmd15_write, fpga_cmd16_write, 0x400634F0, then trigger_offsets_update.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"

void fpga_config_push(void)
{
    fpga_cmd61_write();
    fpga_cmd0c_write();
    fpga_cmd1a_write();
    fpga_cmd15_write();
    fpga_cmd16_write();
    fpga_params_0e_13_write();
    trigger_offsets_update();
}
