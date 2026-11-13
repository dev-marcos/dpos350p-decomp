/*
 * display_init -- stock 0x4003E5D8 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fill the display descriptor (DE2 0x01000000, TCON 0x01C0C000, width, height, 16 bpp, framebuffer), TCON init, DE init, TCON enable, 1 ms, commit.
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

void tcon_init(disp_t *dp);
void de_init(disp_t *dp);
void tcon_enable(disp_t *dp);
void de_commit(disp_t *dp);

void display_init(u32 width, u32 height, u32 fb)
{
    disp_t *dp = g_disp;
    g_disp_ptr = dp;
    dp->width = width;
    dp->bpp = 0x10;
    dp->de_base = 0x01000000u;
    dp->tcon_base = 0x01C0C000u;
    dp->height = height;
    dp->fb = fb;
    tcon_init(dp);
    de_init(g_disp_ptr);
    tcon_enable(g_disp_ptr);
    delay_ms(1);
    de_commit(g_disp_ptr);
}
