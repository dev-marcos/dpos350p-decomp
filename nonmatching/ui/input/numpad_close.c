/*
 * numpad_close -- stock 0x4005FB44 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ui_window_close(0x21); the caller's r1/r2 pass through to its hidden slot argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"

void numpad_close(u32 r0, u32 r1, u32 r2)
{
    (void)r0;
    ui_window_close(0x21, r1, r2);
}
