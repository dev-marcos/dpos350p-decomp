/*
 * func_400606a0_unless_locked -- stock 0x4006068C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, branches to 0x400606A0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/screens.h"

void func_400606a0_unless_locked(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    levels_markers_draw();
}
