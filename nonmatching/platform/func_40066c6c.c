/*
 * local_irq_enable_body -- stock 0x40066C6C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CPSR_c = CPSR & 0x7F: clears the I bit (msr cpsr_c).
 */
#include "hw/platform.h"

void local_irq_enable_body(void)
{
    u32 v;
    __asm__ volatile ("mrs %0, apsr" : "=r" (v));
    v &= 0x7F;
    __asm__ volatile ("msr cpsr_c, %0" : : "r" (v) : "memory");
}
