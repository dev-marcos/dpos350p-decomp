/*
 * runtime_rearm_setup -- stock 0x4003C520 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Re-arm after a settings change: 0x4003BEA8, 0x4003B7B8, 0x40024E14, 0x4003C008, fb_present, write_sys_info_0x6f0_to_fpga, acquisition_prepare, then pulse_fpga_selector_0x08_0_1. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003bea8(void);
void func_4003b7b8(void);
void func_40024e14(void);
void func_4003c008(void);
void fb_present(void);
void write_sys_info_0x6f0_to_fpga(void);
void pulse_fpga_selector_0x08_0_1(void);

void runtime_rearm_setup(void)
{
    func_4003bea8();
    func_4003b7b8();
    func_40024e14();
    func_4003c008();
    fb_present();
    write_sys_info_0x6f0_to_fpga();
    acquisition_prepare();
    pulse_fpga_selector_0x08_0_1();
}
