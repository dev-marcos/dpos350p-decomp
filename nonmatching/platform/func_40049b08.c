/*
 * runtime_event_service -- stock 0x40049B08 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch poll from the acquisition loop: same event rule as touch_service, also sets the serviced flag. (name: Hack ACQUISITION.md; the code services the touch panel)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void runtime_event_service(void)
{
    volatile touch_t *t = g_touch;
    if (t->enabled == 0 || t->busy != 0)
        return;
    gt911_poll();
    if (t->event != 0 || (t->touched != 0 && g_touch_hold != 0)) {
        t->busy = 1;
        t->busy2 = 1;
        t->serviced = 1;
    }
}
