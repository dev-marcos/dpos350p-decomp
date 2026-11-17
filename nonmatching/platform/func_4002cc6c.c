/*
 * sys_rect_redraw_unless_t17 -- stock 0x4002CC6C (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: traces_e10c_enabled, sys_rect_fill, then func_4002c7ac_or_4002c5f8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */

u32 traces_e10c_enabled(void);
void sys_rect_fill(void);
void func_4002c7ac_or_4002c5f8(void);

void sys_rect_redraw_unless_t17(void)
{
    if (TCH8(0x17))
        return;
    traces_e10c_enabled();
    sys_rect_fill();
    func_4002c7ac_or_4002c5f8();
}
