/*
 * ui_window_open -- stock 0x4003CDFC (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Puts window id into the first free stack slot and marks its record open, dirty and animated (+0, +1, +3 = 1); nothing when the 20 slots are full. Then updates the stack top.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/windows.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)

void ui_window_open(u32 id)
{
    u32 i;
    for (i = 0; i < 20; i++) {
        if (WSTK[2 + i] == 0) {
            WSTK[2 + i] = id;
            WIN(id)[0] = 1;
            WIN(id)[1] = 1;
            WIN(id)[3] = 1;
            break;
        }
    }
    ui_window_stack_top_update();
}
