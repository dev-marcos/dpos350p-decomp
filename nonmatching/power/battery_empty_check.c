/*
 * battery_empty_check -- stock 0x4004A3C0 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Battery level 0: stops timer 0, draws the empty-battery image at the screen centre, waits 3 s, powers off.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "power.h"
#include "stock_asm.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "sys_info.h"

void battery_empty_check(void)
{
    u32 cx = SI(plot_x0) + (SI(plot_width) >> 1);
    u32 cy = SI(plot_y0) + (SI(plot_height) >> 1);
    if (SI(battery_level) == 0) {
        unsigned char *p = (unsigned char *)(0x42C2FE74u + 0xDD00);
        timer_stop(0);
        draw_target_default();
        message_box_draw(cx + (signed char)p[0x194], cy + (signed char)p[0x195], *(u32 *)p, 0);
        delay_ms(3000);
        power_off();
    }
}
