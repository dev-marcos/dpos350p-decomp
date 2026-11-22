/* Functions of nonmatching/drivers/spi/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_SPI_H
#define DRIVERS_SPI_H

#include "rt/bits.h"

s32 spi0_clk_init(int ctrl);                                                                     /* 0x40042E8C */
void sys_spi_init(int spi);                                                                      /* 0x40042F00 */
void sys_spi_select(int spi);                                                                    /* 0x4004302C */
void sys_spi_deselect(int spi);                                                                  /* 0x40043088 */
void sys_spi_set_counts(int spi, u32 txn, u32 total);                                            /* 0x400430E8 */
void sys_spi_start_xch(int spi);                                                                 /* 0x40043138 */
int sys_spi_cpu_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len);          /* 0x40043164 */
int sys_spi_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len);              /* 0x40043330 */
int sys_spi_write_then_read(int spi, const unsigned char *tx, int txlen, unsigned char *rx, int rxlen); /* 0x40043360 */
int sys_spi_write_then_write(int spi, const unsigned char *tx0, int len0, const unsigned char *tx1, int len1); /* 0x400433C8 */
void spi0_pins_init(u32 mode);                                                                   /* 0x4004341C */
void spi0_clock_on(void);                                                                        /* 0x4004346C */
void spi0_init(u32 mode);                                                                        /* 0x40043474 */
u32 spi_flash_cmd_read1(u32 dev, u32 cmd);                                                       /* 0x40043494 */
void spi_flash_wait_ready(u32 dev);                                                              /* 0x400434F4 */
int spi_flash_write_enable(u32 dev);                                                             /* 0x40043514 */
int spi_flash_sector_erase(u32 dev, u32 addr);                                                   /* 0x40043564 */
int spi_flash_page_program(u32 dev, u32 addr, const unsigned char *buf, int len);                /* 0x4004372C */
void spi_flash_write_paged(u32 dev, u32 addr, u32 buf, u32 len);                                 /* 0x4004380C */

#endif
