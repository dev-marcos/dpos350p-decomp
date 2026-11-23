/*
 * bode_cursors_draw_gated -- stock 0x40037C28 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, branches to 0x40037C3C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/screens.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */

void bode_cursors_draw_gated(void)
{
    if (TCH8(0x17) == 0)
        bode_cursors_draw();
}
