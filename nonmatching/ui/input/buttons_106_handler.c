/*
 * buttons_106_handler -- stock 0x40055720 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x106, 0x110, 0x11A: the first one released (state 2) is cleared with the touch byte and window 8 is opened.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

void buttons_106_handler(void)
{
    touch_group_0106();
    if (TB(0x106) == 2) {
        TB(0x106) = 0;
    } else if (TB(0x110) == 2) {
        TB(0x110) = 0;
    } else if (TB(0x11A) == 2) {
        TB(0x11A) = 0;
    } else {
        return;
    }
    TB(0) = 0;
    ui_window_open(8);
}
