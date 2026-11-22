/*
 * csi_pins_init -- stock 0x40065EF8 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PE0, PE2, PE3 function 2 (CSI PCLK/HSYNC/VSYNC), PE8..PE15 function 2 (data) via PE_CFG1 = 0x22222222.
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

void csi_pins_init(void)
{
    pio_cfg_pin((void *)0x01C20890u, 0, 2, 0);
    pio_cfg_pin((void *)0x01C20890u, 2, 2, 0);
    pio_cfg_pin((void *)0x01C20890u, 3, 2, 0);
    *(volatile u32 *)(0x01C20890u + 4) = 0x22222222u;
}
