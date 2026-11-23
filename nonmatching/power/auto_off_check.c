/*
 * auto_off_check -- stock 0x4004A1F4 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When auto power-off is on and its deadline passed: draws the shutdown image at the screen centre, waits 3 s and powers off.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "power.h"
#include "stock_asm.h"
#include "ui/screens.h"
#include "sys_info.h"

void auto_off_check(void)
{
    u32 cx = SI(plot_x0) + (SI(plot_width) >> 1);
    u32 cy = SI(plot_y0) + (SI(plot_height) >> 1);
    if (SI(auto_off_minutes) != 0 && SI(auto_off_deadline) < g_tick_count) {
        unsigned char *p = (unsigned char *)(0x42C2FE74u + 0xDB60);
        message_box_draw(cx + (signed char)p[0x194], cy + (signed char)p[0x195], *(u32 *)p, 0);
        delay_ms(3000);
        power_off();
    }
}
