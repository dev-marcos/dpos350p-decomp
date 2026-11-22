/*
 * screenshot_button_poll -- stock 0x40051E24 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For each open window (stack at 0x42E68BCA.., top index byte 0x42E68BC9) whose entry is 1, from the top down: updates touch button 0x42E62040 and, when it is released (state 2), clears it and byte 0x42E61FF8 and saves a screenshot (list_a_save_new).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/input.h"

#define WS8(o) (*(volatile unsigned char *)(0x42E68BC8u + (o)))

void screenshot_button_poll(void)
{
    int i;
    for (i = WS8(1); i >= 0; i--) {
        if (WS8(2 + i) == 1) {
            touch_button_update((volatile unsigned char *)0x42E62040u);
            if (*(volatile unsigned char *)0x42E62040u == 2) {
                *(volatile unsigned char *)0x42E62040u = 0;
                *(volatile unsigned char *)0x42E61FF8u = 0;
                list_a_save_new();
            }
        }
    }
}
