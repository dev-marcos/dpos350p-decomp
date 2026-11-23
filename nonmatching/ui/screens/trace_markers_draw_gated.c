/*
 * trace_markers_draw_gated -- stock 0x40024C4C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, branches to 0x40024C60.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/screens.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */

void trace_markers_draw_gated(void)
{
    if (TCH8(0x17) == 0)
        trace_markers_draw();
}
