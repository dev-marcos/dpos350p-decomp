/*
 * sys_3c_enter -- stock 0x400367A4 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x3D = SYS+0x3C; byte SYS+0x3DA52C = 1; SYS+0x3C = 1; SYS+0x652 = 0; SYS+0x2AA = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void sys_3c_enter(void)
{
    SI(f_03d) = SI(acq_transition_flag);
    SI(f_3da52c) = 1;
    SI(acq_transition_flag) = 1;
    SI(f_652) = 0;
    SI(zoom_on) = 0;
}
