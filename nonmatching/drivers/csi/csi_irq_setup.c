/*
 * csi_irq_setup -- stock 0x40066054 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Pins, ring buffer 0x42DF76D4, IRQ 0x74 -> csi_isr, IRQs on, frame-done interrupt enable.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "drivers/gic.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_irq_setup(void)
{
    csi_pins_init();
    csi_set_buffer_address(0, 0, 0, (u64)0x42DF76D4u);
    irq_register(0x74, csi_isr, 0, 1);
    local_irq_enable();
    csi_int_enable_t(0, 0, 2);
}
