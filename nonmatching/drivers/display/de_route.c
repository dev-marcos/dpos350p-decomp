/*
 * de_route -- stock 0x4006301C (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Blender BLD_CH_RTCTL (+0x101080): channel of a pipe (4 bits per pipe); returns 0.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

int de_route(int pipe, u32 ch)
{
    u32 r = g_disp_ptr->de_base + 0x101000 + 0x80, sh = (u32)pipe << 2;
    de_writel(r, (de_readl(r) & ~(0xFu << sh)) | (ch << sh));
    return 0;
}
