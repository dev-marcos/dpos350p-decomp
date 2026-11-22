/*
 * window_touch_poll -- stock 0x40052098 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: screenshot_button_poll, then the touch handler 0x40051E88 of each window of the stack from the top down, stopping after the first one whose byte +2 in its 14-byte record at 0x42E64520 is 0; finally touch byte +0xC = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

#define WS8(o) (*(volatile unsigned char *)(0x42E68BC8u + (o)))

void window_touch_poll(void)
{
    int i;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    screenshot_button_poll();
    for (i = WS8(1); i >= 0; i--) {
        u32 id = WS8(2 + i);
        window_buttons_dispatch(id);
        if (*(volatile unsigned char *)(0x42E64520u + id * 14 + 2) == 0)
            break;
    }
    *(volatile unsigned char *)(0x42E68BB0u + 0xC) = 0;
}
