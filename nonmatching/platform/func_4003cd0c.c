/*
 * ui_window_stack_top_update -- stock 0x4003CD0C (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Finds the highest occupied slot of the window stack (slots at 0x42E68BC8+2..+0x15) and stores its index at +1 and its window id at +0; unchanged when the stack is empty.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)

void ui_window_stack_top_update(void)
{
    int i;
    for (i = 19; i >= 0; i--) {
        if (WSTK[2 + i]) {
            WSTK[1] = i;
            WSTK[0] = WSTK[2 + i];
            return;
        }
    }
}
