/*
 * acq_wait_done -- stock 0x4003FCC0 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Polls the acquisition-done flag (selector 0x0A); after 2500 polls each further poll also services the touch panel and gives up on a touch event.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void acq_wait_done(void)
{
    int n = 0;
    SI(acq_done) = 0;
    for (;;) {
        acq_done_read();
        if (SI(acq_done) != 0)
            return;
        if (++n < 2500)
            continue;
        runtime_event_service();
        if (g_touch->busy2 != 0)
            return;
    }
}
