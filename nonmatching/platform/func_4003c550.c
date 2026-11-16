/*
 * traces_e10c_enabled -- stock 0x4003C550 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Runs 0x4005E10C for every enabled trace: 0 (CH1 on, SYS+0x3E), then 2 when SYS+0x2AA is set; 1 (CH2 on, SYS+0x102), then 3. When SYS+0x652 is set it only runs 0x4005E10C(0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4005e10c(u32 tr);

void traces_e10c_enabled(void)
{
    if (SYS8(0x652)) {
        func_4005e10c(0);
        return;
    }
    if (SYS8(0x3E)) {
        func_4005e10c(0);
        if (SYS8(0x2AA))
            func_4005e10c(2);
    }
    if (SYS8(0x102)) {
        func_4005e10c(1);
        if (SYS8(0x2AA))
            func_4005e10c(3);
    }
}
