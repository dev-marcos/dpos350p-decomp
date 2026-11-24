/*
 * ui_window_12_redraw -- stock 0x4003D86C (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 12 with the draw function 0x4001E8E0. When anim is set the window is drawn into the offscreen buffer and then revealed into the back buffer with ui_reveal_slide_down; otherwise it is drawn straight into the back buffer.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "shared/framebuffer.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)

void ui_window_12_redraw(u32 anim)
{
    if (anim)
        fb_target_offscreen();
    else
        fb_target_back();
    menu_three_rows_draw();
    if (anim) {
        fb_target_back();
        ui_reveal_slide_down((volatile unsigned short *)WIN(12));
    }
}
