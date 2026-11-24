/*
 * tcon_clock_off -- stock 0x4003E00C (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TCON bus gate (0x01C20064 bit 4), TCON clock (0x01C20118 bit 31) and TCON reset (0x01C202C4 bit 4) off.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

void tcon_clock_off(void)
{
    lcd_writel(0x01C20064u, lcd_readl(0x01C20064u) & ~0x10u);
    lcd_writel(0x01C20118u, lcd_readl(0x01C20118u) & ~0x80000000u);
    lcd_writel(0x01C202C4u, lcd_readl(0x01C202C4u) & ~0x10u);
}
