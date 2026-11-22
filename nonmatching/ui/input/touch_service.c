/*
 * touch_service -- stock 0x40049A9C (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * UI-loop touch poll: a press/release edge, or a held touch when repeat is on (0x42E61FF8), marks an event for the UI.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_service(void)
{
    volatile touch_t *t = g_touch;
    t->serviced = 0;
    if (t->busy != 0)
        return;
    gt911_poll();
    if (t->event != 0 || (t->touched != 0 && g_touch_hold != 0)) {
        t->busy = 1;
        t->busy2 = 1;
    }
    if (t->enabled == 0)
        t->busy2 = 0;
}
