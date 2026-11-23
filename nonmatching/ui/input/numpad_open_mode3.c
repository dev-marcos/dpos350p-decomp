/*
 * numpad_open_mode3 -- stock 0x4005FA34 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * numpad_reset, mode byte +1 = 3, window 0x21 position = (x, y), then ui_window_open(0x21).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/entry_pad.h"

#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))

void numpad_open_mode3(u32 x, u32 y)
{
    numpad_reset();
    NUMPAD(mode) = 3;
    WIN21[4] = y;
    WIN21[3] = x;
    ui_window_open(0x21);
}
