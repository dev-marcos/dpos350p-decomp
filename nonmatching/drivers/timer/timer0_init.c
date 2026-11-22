/*
 * timer0_init -- stock 0x40042C94 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Timer 0 as a 1 kHz tick: IRQs off, timer_setup(0, 1000, 1), register timer0_isr for IRQ 50, start, IRQs on.
 */
#include "hw/drivers.h"
#include "drivers/gic.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"
#include "stock_asm.h"

void timer0_init(void)
{
    irq_disable();
    timer_setup(0, 1000, 1);
    irq_register(0x32, timer0_isr, 0, 1);
    timer_start(0);
    irq_enable();
}
