/*
 * runtime_rearm_setup -- stock 0x4003C520 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Re-arm after a settings change: 0x4003BEA8, 0x4003B7B8, 0x40024E14, 0x4003C008, fb_present, write_sys_info_0x6f0_to_fpga, acquisition_prepare, then pulse_fpga_selector_0x08_0_1. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"

void runtime_rearm_setup(void)
{
    runtime_state_commit_force();
    acquisition_rearm();
    func_40023ea4_or_40024c60();
    acquisition_geometry_setup();
    fb_present();
    write_sys_info_0x6f0_to_fpga();
    acquisition_prepare();
    pulse_fpga_selector_0x08_0_1();
}
