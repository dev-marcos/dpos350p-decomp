/*
 * touch_btn_019c_release -- stock 0x40055C78 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Updates button +0x19C (0x4002FDC4); on its release clears it and the table byte +0, leaves USB mode, targets the back buffer, resets the windows, clears the frame and opens windows 1..5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "usb/core.h"
#include "state/touch_buttons.h"


void touch_btn_019c_release(void)
{
    touch_button_update_019C();
    if (BREC(41, state) != 2)
        return;
    BREC(41, state) = 0;
    BREC_BYTE0 = 0;
    usb_mode_exit();
    fb_target_back();
    ui_windows_reset();
    fb_clear_current();
    ui_windows_open_1_5();
}
