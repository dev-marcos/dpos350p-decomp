/*
 * de_ui_set_size -- stock 0x40063108 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * UI channel overlay size (+0x88) = (h-1)<<16 | (w-1); returns 0.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */
#define g_disp_ptr  (*(disp_t **)0x42E68CB4u)
#define g_disp      ((disp_t *)0x42E68B68u)
#define lcd_timing  ((const u32 *)0x400952B0u)    /* panel timing table (.data), indexed in words */

int de_ui_set_size(int ch, u32 w, u32 h)
{
    u32 r = g_disp_ptr->de_base + ((u32)ch << 12) + 0x102000 + 0x88;
    de_writel(r, 0xFFFEFFFFu + (w + (h << 16)));
    return 0;
}
