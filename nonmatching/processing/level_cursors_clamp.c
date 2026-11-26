/*
 * level_cursors_clamp -- stock 0x400340D4 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Words SYS+0x810 and +0x814 are kept in 12..(u16 SYS+0xA) - 15 (the first is always stored, the second only when changed).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void level_cursors_clamp(void)
{
    int v = (int)SI(cursor_y1), m;
    if (v < 12) {
        v = 12;
    } else {
        m = SI(plot_height) - 15;
        if (m < v)
            v = m;
    }
    SI(cursor_y1) = v;
    v = (int)SI(cursor_y2);
    if (v < 12) {
        SI(cursor_y2) = 12;
    } else {
        m = SI(plot_height) - 15;
        if (m < v)
            SI(cursor_y2) = m;
    }
}
