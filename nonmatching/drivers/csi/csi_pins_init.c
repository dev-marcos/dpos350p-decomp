/*
 * csi_pins_init -- stock 0x40065EF8 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PE0, PE2, PE3 function 2 (CSI PCLK/HSYNC/VSYNC), PE8..PE15 function 2 (data) via PE_CFG1 = 0x22222222.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "drivers/pio.h"
#include "stock_asm.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)

void csi_pins_init(void)
{
    pio_cfg_pin((void *)0x01C20890u, 0, 2, 0);
    pio_cfg_pin((void *)0x01C20890u, 2, 2, 0);
    pio_cfg_pin((void *)0x01C20890u, 3, 2, 0);
    *(volatile u32 *)(0x01C20890u + 4) = 0x22222222u;
}
