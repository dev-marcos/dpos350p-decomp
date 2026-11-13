/*
 * csi_irq_setup -- stock 0x40066054 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Pins, ring buffer 0x42DF76D4, IRQ 0x74 -> csi_isr, IRQs on, frame-done interrupt enable.
 */
#include "hw/platform.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)
u32 csi_readl(u32 addr);
void csi_writel(u32 addr, u32 v);
void csi_setbits(u32 addr, u32 bits);
void csi_clrbits(u32 addr, u32 bits);
void csi_clrsetbits(u32 addr, u32 clr, u32 set);
void dcache_clean_inv_all(void);
void csi_writel_t(u32 addr, u32 v);
u32 csi_readl_t(u32 addr);

void csi_pins_init(void);
void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr);
void irq_register(int irq, void (*handler)(void), int prio, int enable);
void local_irq_enable(void);
void csi_isr(void);
void csi_int_enable_t(u32 sel, u32 ch, u32 bits);

void csi_irq_setup(void)
{
    csi_pins_init();
    csi_set_buffer_address(0, 0, 0, (u64)0x42DF76D4u);
    irq_register(0x74, csi_isr, 0, 1);
    local_irq_enable();
    csi_int_enable_t(0, 0, 2);
}
