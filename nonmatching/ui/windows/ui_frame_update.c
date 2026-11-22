/*
 * ui_frame_update -- stock 0x4003CF58 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ui_window_redraw_dirty, battery_service, then, unless the byte 0x42E68BC7 is set, fb_present and the halfword 0x42E68BC4 = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "power.h"
#include "ui/draw.h"
#include "ui/windows.h"

void ui_frame_update(void)
{
    ui_window_redraw_dirty();
    battery_service();
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    fb_present();
    *(volatile unsigned short *)0x42E68BC4u = 0;
}
