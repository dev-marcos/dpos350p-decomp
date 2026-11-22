/*
 * func_40023ea4_unless_t17 -- stock 0x40023E90 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, branches to 0x40023EA4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/screens.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */

void func_40023ea4_unless_t17(void)
{
    if (TCH8(0x17) == 0)
        plot_markers_draw();
}
