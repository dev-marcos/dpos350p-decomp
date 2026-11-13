/*
 * de_ui_set_layer -- stock 0x40063070 (152 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * UI channel ch (+0x102000 + ch * 0x1000), layer l (32 bytes each): attribute = enable | format << 8, size = (h-1)<<16 | (w-1), coordinate, pitch, top address; returns 0.
 */
#include "hw/platform.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */
#define g_disp_ptr  (*(disp_t **)0x42E68CB4u)
#define g_disp      ((disp_t *)0x42E68B68u)
#define lcd_timing  ((const u32 *)0x400952B0u)    /* panel timing table (.data), indexed in words */
u32 lcd_readl(u32 addr);
void lcd_writel(u32 addr, u32 v);
u32 de_readl(u32 addr);
void de_writel(u32 addr, u32 v);
void ccu_gate_on(s32 code);
void ccu_gate_off(s32 code);
void delay_ms(u32 ms);
void *memset_w(void *d, int v, unsigned int n);
unsigned long long __aeabi_llsl(unsigned long long v, int sh);

int de_ui_set_layer(int ch, int layer, u32 x, u32 y, u32 w, u32 h, u32 addr, u32 pitch, signed char fmt)
{
    u32 r = g_disp_ptr->de_base + ((u32)ch << 12) + 0x102000 + ((u32)layer << 5);
    de_writel(r, 1 | ((u32)(int)fmt << 8));
    de_writel(r + 4, 0xFFFEFFFFu + (w + (h << 16)));
    de_writel(r + 8, x + (y << 16));
    de_writel(r + 0xC, pitch);
    de_writel(r + 0x10, addr);
    return 0;
}
