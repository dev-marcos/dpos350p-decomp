/*
 * tcon_enable -- stock 0x4003E2E0 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TCON global enable (bit 31).
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

void tcon_enable(disp_t *dp)
{
    u32 r = dp->tcon_base;
    lcd_writel(r, lcd_readl(r) | 0x80000000u);
}
