/*
 * numpad_open_mode6 -- stock 0x4005FB00 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * numpad_reset, mode byte +1 = 6, window 0x21 position = (x, y), then ui_window_open(0x21).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))
void numpad_reset(void);
void ui_window_open(u32 id);

void numpad_open_mode6(u32 x, u32 y)
{
    numpad_reset();
    PAD(1) = 6;
    WIN21[4] = y;
    WIN21[3] = x;
    ui_window_open(0x21);
}
