/*
 * cursor_27e_clamp -- stock 0x40063484 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clamps the s16 SYS+0x27E to m .. height - m (m = u16 SYS+0x1AE, height SYS+0xA), or to 2m .. height - 2m when byte SYS+0x2AA is set; the upper limit is tested first.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void cursor_27e_clamp(void)
{
    int m = SI(f_1ae), h = SI(plot_height), v = (short)SI(trigger_level);
    if (SI(zoom_on) == 0) {
        if (h - m < v)
            v = h - m;
        else if (v < m)
            v = m;
    } else {
        if (h - 2 * m < v)
            v = h - 2 * m;
        else if (v < 2 * m)
            v = 2 * m;
    }
    SI(trigger_level) = v;
}
