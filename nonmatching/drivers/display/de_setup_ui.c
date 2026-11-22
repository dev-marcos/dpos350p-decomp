/*
 * de_setup_ui -- stock 0x4006314C (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Route pipe 0 to UI channel 2, enable pipe 0, layer 0 of channel 2 = the framebuffer (RGB565, format 0x0A, pitch 2 * width), overlay size; returns 0.
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

int de_route(int pipe, u32 ch);
int de_pipe_enable(int pipe, int on);
int de_ui_set_layer(int ch, int layer, u32 x, u32 y, u32 w, u32 h, u32 addr, u32 pitch, signed char fmt);
int de_ui_set_size(int ch, u32 w, u32 h);

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
