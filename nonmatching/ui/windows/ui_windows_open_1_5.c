/*
 * ui_windows_open_1_5 -- stock 0x4003D6AC (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Opens windows 1 to 5 in order.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ui_window_open(u32 id);

void ui_windows_open_1_5(void)
{
    ui_window_open(1);
    ui_window_open(2);
    ui_window_open(3);
    ui_window_open(4);
    ui_window_open(5);
}
