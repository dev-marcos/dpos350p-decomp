/*
 * ch2_frontend_apply -- stock 0x4004AC04 (160 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CH2 counterpart of ch1_frontend_apply: selector 0x2B (copy in +0xBC9), jobs 9 and 11, job 13 (position +0x120), commit 0xB6, selector 0x2D.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void fpga_write_sel_0x2d(void);

void ch2_frontend_apply(void)
{
    int changed = 0;
    if (SYS8(0xBC9) != SYS8(0x105)) {
        u32 v;
        SYS8(0xBC9) = SYS8(0x105);
        fpga_selector_command_write(0x2B);
        v = SYS8(0xBC9);
        changed = 1;
        if (v == 12) v = 11;
        fpga_data_byte_write(v);
    }
    fpga_param_updater(9, 4, CAL_OBJ[1], CAL_OBJ[1]);
    fpga_param_updater(11, 4, CAL_OBJ[3], CAL_OBJ[3]);
    fpga_param_updater(13, 4, SYS32(0x120), SYS32(0x120));
    smec_commit(0xB6);
    fpga_write_sel_0x2d();
    if (changed)
        delay_ms(60);
}
