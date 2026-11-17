/*
 * cursor_27e_clamp -- stock 0x40063484 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clamps the s16 SYS+0x27E to m .. height - m (m = u16 SYS+0x1AE, height SYS+0xA), or to 2m .. height - 2m when byte SYS+0x2AA is set; the upper limit is tested first.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void cursor_27e_clamp(void)
{
    int m = SYS16(0x1AE), h = SYS16(0xA), v = (short)SYS16(0x27E);
    if (SYS8(0x2AA) == 0) {
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
    SYS16(0x27E) = v;
}
