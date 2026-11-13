/*
 * de_route -- stock 0x4006301C (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Blender BLD_CH_RTCTL (+0x101080): channel of a pipe (4 bits per pipe); returns 0.
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

int de_route(int pipe, u32 ch)
{
    u32 r = g_disp_ptr->de_base + 0x101000 + 0x80, sh = (u32)pipe << 2;
    de_writel(r, (de_readl(r) & ~(0xFu << sh)) | (ch << sh));
    return 0;
}
