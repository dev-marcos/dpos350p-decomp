/*
 * csi_full_init -- stock 0x400660C8 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clock, base 0x01CB4000, reset, enable, descriptors (rate), IRQ setup. (Stock name is_a_generic_delay_helper.)
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

void csi_clk_init(void);
int csi_set_base_addr(u32 sel, u32 addr);
void csi_reset(u32 sel);
void csi_enable_t(u32 sel);
int csi_init_structs(u32 rate);
void csi_irq_setup(void);

void csi_full_init(u32 rate)
{
    csi_clk_init();
    csi_set_base_addr(0, 0x01CB4000u);
    csi_reset(0);
    csi_enable_t(0);
    csi_init_structs(rate);
    csi_irq_setup();
}
