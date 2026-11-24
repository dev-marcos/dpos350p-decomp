/*
 * csi_full_init -- stock 0x400660C8 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clock, base 0x01CB4000, reset, enable, descriptors (rate), IRQ setup. (Stock name is_a_generic_delay_helper.)
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

void csi_full_init(u32 rate)
{
    csi_clk_init();
    csi_set_base_addr(0, 0x01CB4000u);
    csi_reset(0);
    csi_enable_t(0);
    csi_init_structs(rate);
    csi_irq_setup();
}
