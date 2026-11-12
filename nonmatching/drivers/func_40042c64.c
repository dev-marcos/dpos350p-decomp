/*
 * timer0_isr -- stock 0x40042C64 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Timer 0 interrupt: acknowledge (IRQ_STA |= 1) and increment the tick counter at 0x42E68CD8.
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

extern volatile u32 g_tick_count;   /* 0x42E68CD8 */

void timer0_isr(void)
{
    writel_d(TMR_IRQ_STA, readl_d(TMR_IRQ_STA) | 1);
    g_tick_count = g_tick_count + 1;
}
