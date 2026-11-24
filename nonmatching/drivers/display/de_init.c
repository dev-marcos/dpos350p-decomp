/*
 * de_init -- stock 0x4003E314 (708 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * DE clocks, mixer 0 on with the panel size, channel/blender register blocks cleared, blender routing 0x03010301, four pipes with opaque black fill, post-processing blocks (CSC, peaking, ...) off, then the UI layer setup.
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

void de_init(disp_t *dp)
{
    u32 de = dp->de_base, bld = de + 0x101000, glb = de + 0x100000;
    u32 size = (dp->width - 1) | ((dp->height - 1) << 16);
    int i;
    ccu_gate_on(0x0C8C588C);
    lcd_writel(0x01C20104u, lcd_readl(0x01C20104u) | 0x80000000u);
    lcd_writel(de + 8, lcd_readl(de + 8) | 1);
    lcd_writel(de, lcd_readl(de) | 1);
    lcd_writel(de + 4, lcd_readl(de + 4) | 1);
    lcd_writel(de + 0x10, lcd_readl(de + 0x10) & ~1u);
    lcd_writel(glb, 1);
    lcd_writel(glb + 4, 0);
    lcd_writel(glb + 8, 0);
    lcd_writel(glb + 0xC, size);
    memset_w((void *)(dp->de_base + 0x102000), 0, 0x100);
    for (i = 1; i <= 3; i++)
        memset_w((void *)(dp->de_base + 0x1000 * i + 0x102000), 0, 0x8C);
    memset_w((void *)bld, 0, 0x100);
    lcd_writel(bld, 0);
    lcd_writel(bld + 0x80, 0);
    lcd_writel(bld + 0x84, 0);
    lcd_writel(bld + 0x88, 0);
    lcd_writel(bld + 0x90, 0x03010301u);
    lcd_writel(bld + 0x94, 0x03010301u);
    lcd_writel(bld + 0x8C, size);
    lcd_writel(bld + 0xFC, 0);
    lcd_writel(bld + 0xB0, 0);
    for (i = 0; i < 4; i++) {
        lcd_writel(bld + 4 + 0x10 * i, 0xFF000000u);
        lcd_writel(bld + 8 + 0x10 * i, size);
    }
    lcd_writel(dp->de_base + 0x120000, 0);
    lcd_writel(dp->de_base + 0x130000, 0);
    lcd_writel(dp->de_base + 0x140000, 0);
    lcd_writel(dp->de_base + 0x150000, 0);
    lcd_writel(dp->de_base + 0x1A0000, 0);
    lcd_writel(dp->de_base + 0x1A2000, 0);
    lcd_writel(dp->de_base + 0x1A4000, 0);
    lcd_writel(dp->de_base + 0x1A6000, 0);
    lcd_writel(dp->de_base + 0x1A8000, 0);
    lcd_writel(dp->de_base + 0x1AA000, 0);
    lcd_writel(dp->de_base + 0x1B0000, 0);
    de_setup_ui();
}
