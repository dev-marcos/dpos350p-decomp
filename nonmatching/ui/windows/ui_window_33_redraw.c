/*
 * ui_window_33_redraw -- stock 0x4003DD5C (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Redraws window 33 with the draw function 0x4005FF88. When anim is set the window is drawn into the offscreen buffer and then revealed into the back buffer with ui_reveal_slide_down when the byte 0x42E68A8A is set, else ui_reveal_bottom_up; otherwise it is drawn straight into the back buffer.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)
void fb_target_back(void);
void fb_target_offscreen(void);
void ui_reveal_slide_down(volatile unsigned short *r);
void ui_reveal_bottom_up(volatile unsigned short *r);

void numpad_draw(void);

void ui_window_33_redraw(u32 anim)
{
    if (anim)
        fb_target_offscreen();
    else
        fb_target_back();
    numpad_draw();
    if (anim) {
        fb_target_back();
        if (*(volatile unsigned char *)(0x42E68A88u + 2))
            ui_reveal_slide_down((volatile unsigned short *)WIN(33));
        else
            ui_reveal_bottom_up((volatile unsigned short *)WIN(33));
    }
}
