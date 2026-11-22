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

void battery_empty_check(void)
{
    u32 cx = SYS16(4) + (SYS16(8) >> 1);
    u32 cy = SYS16(6) + (SYS16(0x0A) >> 1);
    if (SYS8(0x778) == 0) {
        unsigned char *p = (unsigned char *)(0x42C2FE74u + 0xDD00);
        timer_stop(0);
        draw_target_default();
        message_box_draw(cx + (signed char)p[0x194], cy + (signed char)p[0x195], *(u32 *)p, 0);
        delay_ms(3000);
        power_off();
    }
}
