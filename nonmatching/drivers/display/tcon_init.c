/*
 * tcon_init -- stock 0x4003E064 (636 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PLL_VIDEO from the panel table (N, M), TCON clocks on, TCON0 control/clock divider, BASIC0..3 timing (active size, horizontal/vertical total and back porch, sync widths), FRM dither tables, FRM mode (18 bpp: off), IO polarity from the table flags.
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

void tcon_init(disp_t *dp)
{
    const u32 *t = lcd_timing;
    u32 tc = dp->tcon_base, v;
    lcd_writel(0x01C20010u, lcd_readl(0x01C20010u) & ~0x80000000u);
    lcd_writel(0x01C20010u, ((t[0x40 / 4] - 1) | ((t[0x3C / 4] - 1) << 8)) | 0x3000000);
    lcd_writel(0x01C20010u, lcd_readl(0x01C20010u) | 0x80000000u);
    ccu_gate_on(0x0C845884);
    lcd_writel(0x01C20118u, 0x80000000u);
    delay_ms(1);
    lcd_writel(0x01C0C040u, lcd_readl(0x01C0C040u) & ~1u);
    lcd_writel(0x01C0C040u, (((t[0x18 / 4] + t[0x1C / 4] + t[0x20 / 4]) << 4) & 0x1F0) | 0x80000000u);
    lcd_writel(0x01C0C044u, t[0x44 / 4] | 0xF0000000u);
    lcd_writel(tc + 0x48, (t[0x14 / 4] - 1) | ((t[0x04 / 4] - 1) << 16));
    v = t[0x0C / 4] + t[0x10 / 4];
    lcd_writel(tc + 0x4C, (v - 1) | ((v + t[0x34 / 4] * t[0x04 / 4] + t[0x08 / 4] - 1) << 16));
    v = t[0x1C / 4] + t[0x20 / 4];
    lcd_writel(tc + 0x50, (v - 1) | ((v + t[0x18 / 4] + t[0x14 / 4]) << 17));
    lcd_writel(tc + 0x54, (t[0x20 / 4] - 1) | ((t[0x10 / 4] - 1) << 16));
    lcd_writel(0x01C0C040u, lcd_readl(0x01C0C040u) | 0x800000);
    lcd_writel(tc + 0x58, 0);
    lcd_writel(tc + 0x60, 0);
    lcd_writel(tc + 0x14, 0x11111111u);
    lcd_writel(tc + 0x18, 0x11111111u);
    lcd_writel(tc + 0x1C, 0x11111111u);
    lcd_writel(tc + 0x20, 0x11111111u);
    lcd_writel(tc + 0x24, 0x11111111u);
    lcd_writel(tc + 0x28, 0x11111111u);
    lcd_writel(tc + 0x2C, 0x01010000u);
    lcd_writel(tc + 0x30, 0x15151111u);
    lcd_writel(tc + 0x34, 0x57575555u);
    lcd_writel(tc + 0x38, 0x7F7F7777u);
    lcd_writel(tc + 0x10, dp->bpp == 0x12 ? 0x80000000u : 0x80000050u);
    v = t[0x24 / 4] == 0 ? 0x12000000u : 0x10000000u;
    if (t[0x28 / 4] == 0) v |= 0x1000000;
    if (t[0x2C / 4] == 0) v |= 0x08000000;
    if (t[0x30 / 4] == 0) v |= 0x04000000;
    lcd_writel(tc + 0x88, v);
    lcd_writel(tc + 0x8C, 0);
}
