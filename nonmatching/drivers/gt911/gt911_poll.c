/*
 * gt911_poll -- stock 0x400498E4 (340 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Every 15 ms: GT911 status 0x814E (low nibble = points), first point at 0x8150 (clamped to 1022x600), status cleared; press/release edges latched with their points.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/gt911.h"

void gt911_poll(void)
{
    unsigned char st[4], pt[4];
    u32 x, y, touched;
    volatile touch_t *t = g_touch;

    if (!(g_touch_last + 15 < g_tick_count))
        return;
    g_touch_last = g_tick_count;
    gt911_read16(0x814E, st, 1);
    {
        u32 status = st[0];
        st[0] = 0;
        touched = 0;
        if (status & 0xF) {
            gt911_read16(0x8150, pt, 4);
            gt911_write16(0x814E, st, 1);
            x = pt[0] + (pt[1] << 8);
            y = pt[2] + (pt[3] << 8);
            touched = 1;
            if (x > 0x3FE) x = 0x3FE;
            if (y > 0x258) y = 0x258;
        } else {
            gt911_write16(0x814E, st, 1);
            x = 0;
            y = 0;
        }
    }
    t->x = x;
    t->touched = touched;
    t->y = y;
    if (t->touched == 1 && t->prev == 0) {
        t->down_x = t->x;
        t->down_y = t->y;
        t->event = 1;
    } else if (t->touched == 0 && t->prev == 1) {
        t->up_x = t->prev_x;
        t->up_y = t->prev_y;
        t->event = 2;
    }
    t->prev = t->touched;
    t->prev_x = t->x;
    t->prev_y = t->y;
}
