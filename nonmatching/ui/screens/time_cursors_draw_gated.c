/*
 * time_cursors_draw_gated -- stock 0x40028350 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, branches to 0x40028364.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/screens.h"

void time_cursors_draw_gated(void)
{
    if (g_touch->serviced == 0)
        time_cursors_draw();
}
