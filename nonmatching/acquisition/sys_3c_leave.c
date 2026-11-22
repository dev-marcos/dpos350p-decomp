/*
 * sys_3c_leave -- stock 0x400367D0 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Byte SYS+0x3DA52C = 0; SYS+0x3C = SYS+0x3D.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void sys_3c_leave(void)
{
    u32 v = SYS8(0x3D);
    SYS8(0x3DA52C) = 0;
    SYS8(0x3C) = v;
}
