/*
 * timer0_init -- stock 0x40042C94 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Timer 0 as a 1 kHz tick: IRQs off, timer_setup(0, 1000, 1), register timer0_isr for IRQ 50, start, IRQs on.
 */
#include "hw/drivers.h"

u32 readl_d(u32 addr);
void writel_d(u32 addr, u32 v);

void irq_disable(void);                                     /* 0x40066B8C */
void irq_enable(void);                                      /* 0x40066B88 */
void irq_register(u32 irq, void (*fn)(void), u32 a, u32 b); /* 0x40066BD4 */
void timer_setup(u32 n, u32 interval, u32 irq);
void timer_start(u32 n);
void timer0_isr(void);

void timer0_init(void)
{
    irq_disable();
    timer_setup(0, 1000, 1);
    irq_register(0x32, timer0_isr, 0, 1);
    timer_start(0);
    irq_enable();
}
