/*
 * tcon_clock_off -- stock 0x4003E00C (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TCON bus gate (0x01C20064 bit 4), TCON clock (0x01C20118 bit 31) and TCON reset (0x01C202C4 bit 4) off.
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

void tcon_clock_off(void)
{
    lcd_writel(0x01C20064u, lcd_readl(0x01C20064u) & ~0x10u);
    lcd_writel(0x01C20118u, lcd_readl(0x01C20118u) & ~0x80000000u);
    lcd_writel(0x01C202C4u, lcd_readl(0x01C202C4u) & ~0x10u);
}
