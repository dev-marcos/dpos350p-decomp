/*
 * time_cursors_clamp -- stock 0x4003407C (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Words SYS+0x808 and +0x80C are kept in 12..(u16 SYS+8) - 12 (the first is always stored, the second only when changed).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void time_cursors_clamp(void)
{
    int v = (int)SI(cursor_x1), m;
    if (v < 12) {
        v = 12;
    } else {
        m = SI(plot_width) - 12;
        if (m < v)
            v = m;
    }
    SI(cursor_x1) = v;
    v = (int)SI(cursor_x2);
    if (v < 12) {
        SI(cursor_x2) = 12;
    } else {
        m = SI(plot_width) - 12;
        if (m < v)
            SI(cursor_x2) = m;
    }
}
