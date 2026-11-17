/*
 * zoom_views_update -- stock 0x40041F5C (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, and only when SYS+0x2AA, SYS+0x28 and SYS+0x652 are all 0: 0x40041BA0, 0x40041D38, 0x40041E34.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40041ba0(void);
void fft_db_traces_scale(void);
void fft_traces_draw(void);

void zoom_views_update(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    if (SYS8(0x2AA) || SYS8(0x28) || SYS8(0x652))
        return;
    func_40041ba0();
    fft_db_traces_scale();
    fft_traces_draw();
}
