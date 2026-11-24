/*
 * de_pipe_enable -- stock 0x40062FAC (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Blender BLD_FILL_COLOR_CTL (+0x101000): pipe bit (8 + pipe) on or off; returns 0.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

int de_pipe_enable(int pipe, int on)
{
    disp_t *dp = g_disp_ptr;
    u32 bit = (u32)__aeabi_llsl(1, pipe + 8), r = dp->de_base + 0x101000;
    if (on) de_writel(r, bit | de_readl(r));
    else de_writel(r, de_readl(r) & ~bit);
    return 0;
}
