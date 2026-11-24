/* Driver state and register blocks shared by the CSI, display (DE / TCON) and SD/MMC
 * drivers (build 2025-06-19, see docs/DRIVERS.md). */
#ifndef SHARED_DRIVERS_H
#define SHARED_DRIVERS_H

#define CCU 0x01C20000u
#define PE ((pio_port_t *)PIO_PE)
#define PG ((pio_port_t *)PIO_PG)
#define PLL_LOCK 0x10000000u
#define csi_base_tbl ((u32 *)0x42E68C50u)
#define g_disp ((disp_t *)0x42E68B68u)
#define g_disp_ptr (*(disp_t **)0x42E68CB4u)
#define lcd_timing ((const u32 *)0x400952B0u)                /* panel timing table (.data), indexed in words */
#define mmc_capacity (*(u32 *)0x42E68CC0u)
#define mmc_card_type (*(int *)0x42E68CB8u)                  /* 0 none, 1 SDHC, 2 SD (byte addressed), 3 MMC */
#define mmc_dma_buf ((unsigned char *)0x42E68B28u)
#define mmc_rca (*(u32 *)0x42E68CBCu)

#endif /* SHARED_DRIVERS_H */
