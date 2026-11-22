/*
 * runtime_sweep_path -- stock 0x4004191C (176 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Roll/sweep acquisition: only for the run modes 1, 2, 3, 5 and 0x13 (byte 0x42E68BC8) with no touch activity; re-arms, then captures and processes record by record while SYS+0x700 < SYS16+8, stopping (SYS+0x38 = 1) on a touch. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void runtime_rearm_setup(void);
void one_runtime_capture_step(void);
void runtime_state_commit_copy(void);
void per_channel_processing(void);
void runtime_counter_progress_advance(void);
void write_sys_info_0x6f0_to_fpga(void);
void pulse_fpga_selector_0x08_0_1(void);

void runtime_sweep_path(void)
{
    volatile unsigned char *t = (volatile unsigned char *)0x42E68BB0u;
    u32 mode = t[0x18];
    if (mode != 1 && mode != 2 && mode != 3 && mode != 5 && mode != 0x13)
        return;
    if (t[0x17] != 0 || t[0x15] != 0 || t[0x0D] != 0)
        return;
    runtime_rearm_setup();
    while (SYS32(0x700) < SYS16(8)) {
        one_runtime_capture_step();
        runtime_state_commit_copy();
        per_channel_processing();
        runtime_counter_progress_advance();
        write_sys_info_0x6f0_to_fpga();
        pulse_fpga_selector_0x08_0_1();
        runtime_event_service();
        if (t[0x15] != 0) {
            SYS8(0x38) = 1;
            return;
        }
    }
}
