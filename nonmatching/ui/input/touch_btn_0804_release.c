/*
 * touch_btn_0804_release -- stock 0x40054F08 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Updates button +0x804 (0x4002FB48); on its release (state 2) clears it and the table byte +0, then opens window 0x17.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"

#define BTN8(o)  (*(volatile unsigned char *)(0x42E61FF8u + (o)))   /* button table */

void touch_btn_0804_release(void)
{
    touch_button_update_0804();
    if (BTN8(0x804) != 2)
        return;
    BTN8(0x804) = 0;
    BTN8(0) = 0;
    ui_window_open(0x17);
}
