/*
 * acq_wait_done -- stock 0x4003FCC0 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Polls the acquisition-done flag (selector 0x0A); after 2500 polls each further poll also services the touch panel and gives up on a touch event.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void acq_wait_done(void)
{
    int n = 0;
    SYS8(0x38) = 0;
    for (;;) {
        acq_done_read();
        if (SYS8(0x38) != 0)
            return;
        if (++n < 2500)
            continue;
        runtime_event_service();
        if (g_touch->busy2 != 0)
            return;
    }
}
