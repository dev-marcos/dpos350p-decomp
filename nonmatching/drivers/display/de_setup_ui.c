/*
 * de_setup_ui -- stock 0x4006314C (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Route pipe 0 to UI channel 2, enable pipe 0, layer 0 of channel 2 = the framebuffer (RGB565, format 0x0A, pitch 2 * width), overlay size; returns 0.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

int de_setup_ui(void)
{
    disp_t *dp;
    de_route(0, 2);
    de_pipe_enable(0, 1);
    dp = g_disp_ptr;
    de_ui_set_layer(2, 0, 0, 0, dp->width, dp->height, dp->fb, dp->width << 1, 0x0A);
    dp = g_disp_ptr;
    de_ui_set_size(2, dp->width, dp->height);
    return 0;
}
