/*
 * touch_moved_flag -- stock 0x4006904C (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x42E68998 byte = 0x12 when the touch block's second point is not (0, 0), else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void touch_moved_flag(void)
{
    volatile short *t = (volatile short *)0x42E68BB0u;
    if (t[2] == 0 && t[3] == 0)
        *(volatile unsigned char *)0x42E68998u = 0;
    else
        *(volatile unsigned char *)0x42E68998u = 0x12;
}
