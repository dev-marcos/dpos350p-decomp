/*
 * ch1_frontend_apply -- stock 0x4004AAC0 (160 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CH1: V/div index to selector 0x28 when it changed (12 sent as 11, copy in +0xBC8), jobs 8 and 10 (calibration aux values), job 12 (position), commit 0xB5, selector 0x2A; 60 ms after a V/div change.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "frontend.h"
#include "sys_info.h"

void ch1_frontend_apply(void)
{
    int changed = 0;
    if (SI(f_bc8) != SI(ch1_vdiv_index)) {
        u32 v;
        SI(f_bc8) = SI(ch1_vdiv_index);
        fpga_selector_command_write(0x28);
        v = SI(f_bc8);
        changed = 1;
        if (v == 12) v = 11;
        fpga_data_byte_write(v);
    }
    fpga_param_updater(8, 4, CAL_OBJ[0], CAL_OBJ[0]);
    fpga_param_updater(10, 4, CAL_OBJ[2], CAL_OBJ[2]);
    fpga_param_updater(12, 4, SI(ch1_offset), SI(ch1_offset));
    smec_commit(0xB5);
    fpga_write_sel_0x2a();
    if (changed)
        delay_ms(60);
}
