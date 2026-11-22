/*
 * acquisition_dispatcher -- stock 0x400419CC (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Acquisition entry per main-loop pass: nothing while touch byte +0x17 is set; SYS+0x38 = 0 when SYS+0x3C (stop) is set; otherwise the full acquisition (SYS+0x28 = 0) or the sweep path, then runtime_state_commit_copy. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void acquisition_dispatcher(void)
{
    volatile unsigned char *t = (volatile unsigned char *)0x42E68BB0u;
    if (t[0x17] != 0)
        return;
    if (SYS8(0x3C) != 0) {
        SYS8(0x38) = 0;
        return;
    }
    if (SYS8(0x28) == 0)
        full_acquisition_veneer();
    else
        runtime_sweep_path();
    runtime_state_commit_copy();
}
