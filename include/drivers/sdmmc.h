/* Functions of nonmatching/drivers/sdmmc/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_SDMMC_H
#define DRIVERS_SDMMC_H

#include "rt/bits.h"

void dcache_clean_inv_all_t(void);                                                               /* 0x4003E6F4 */
void mmc_delay_us(u32 us);                                                                       /* 0x400675A8 */
void mmc_delay_ms(u32 ms);                                                                       /* 0x400675AC */
void sdmmc_pins_init(int ctrl);                                                                  /* 0x400675B0 */
u32 mmc_base(u32 n);                                                                             /* 0x4006763C */
int mmc_check_range(u32 start, u32 count);                                                       /* 0x40067678 */
int mmc_wait_rint(u32 n, u32 timeout, u32 mask);                                                 /* 0x400676BC */
int mmc_update_clk(u32 n);                                                                       /* 0x4006773C */
s32 mmc_periph0_freq(void);                                                                      /* 0x400677A8 */
int mmc_set_mod_clk(u32 n, u32 hz);                                                              /* 0x400677E4 */
int mmc_clk_lowpower_on(u32 n);                                                                  /* 0x400678B0 */
int mmc_clk_enable(u32 n);                                                                       /* 0x400678D8 */
int mmc_clk_disable(u32 n);                                                                      /* 0x40067918 */
int mmc_gctrl_clr_bit8(u32 n);                                                                   /* 0x40067958 */
int mmc_config_clock(u32 n, u32 hz);                                                             /* 0x4006797C */
int mmc_reset(u32 n);                                                                            /* 0x40067A04 */
int mmc_set_bus_width(u32 n, u32 ctype);                                                         /* 0x40067CFC */
int mmc_idma_status(u32 n);                                                                      /* 0x40067D1C */
void mmc_hw_reset(u32 n);                                                                        /* 0x400680B0 */
int mmc_read_blocks(u32 n, u32 sector, u32 count, u32 *buf);                                     /* 0x4006866C */
int mmc_write_blocks(u32 n, u32 sector, u32 count, u32 *buf);                                    /* 0x40068794 */
void dcache_clean_inv_mva(u32 addr);                                                             /* 0x400688BC */
void dcache_flush_range_se(u32 start, u32 end);                                                  /* 0x400688C8 */
void dcache_flush_range(u32 addr, u32 len);                                                      /* 0x400688FC */
int mmc_card_init(u32 n);                                                                        /* 0x40068918 */

#endif
