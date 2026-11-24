/*
 * ui_window_14_redraw -- stock 0x4003D8DC (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 14 with the draw function 0x4001F5A8. When anim is set the window is drawn into the offscreen buffer and then revealed into the back buffer with ui_reveal_bottom_up; otherwise it is drawn straight into the back buffer.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "shared/framebuffer.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)

void ui_window_14_redraw(u32 anim)
{
    if (anim)
        fb_target_offscreen();
    else
        fb_target_back();
    ch2_menu_draw();
    if (anim) {
        fb_target_back();
        ui_reveal_bottom_up((volatile unsigned short *)WIN(14));
    }
}
