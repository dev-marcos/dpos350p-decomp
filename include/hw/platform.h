/* Platform drivers of the stock firmware: GIC/IRQ, UART + debug printf, SPI, SD/MMC, CSI.
 * Addresses are STATIC-CONFIRMED from the stock code; names are DERIVED (see docs/PLATFORM.md). */
#ifndef HW_PLATFORM_H
#define HW_PLATFORM_H
#include "hw/v3s.h"
#include "image.h"

#define REG16(a) (*(volatile unsigned short *)(a))

#define STR(a) ((const char *)IMG(a))  /* a text of the image: STR(4008e724), see image.h */

/* ---- GIC (0x01C80000: distributor +0x1000, CPU interface +0x2000) */
typedef struct {
    u32 base_lo, base_hi;   /* 64-bit base, loaded with LDRD */
    int irq_base;           /* first interrupt id handled */
    int nr;                 /* number of table entries (0x9C) */
} gic_t;
#define g_gic          (*(gic_t *)0x42E68BF0u)
#define irq_table      ((void (**)(void))0x42E67954u)

/* ---- UART / console */
#define g_uart_port    (*(int *)0x42E68CDCu)

/* ---- SPI (controller n at 0x01C68000 + n * 0x1000) */
#define SPI_BASE(n)    (0x01C68000u + ((u32)(n) << 12))
#define spi_dual_mode  (*(int *)0x42E68CC4u)     /* BCC bits 31:28 */
#define spi_cs_index   (*(int *)0x42E68CC8u)
#define spi_soft_cs    (*(int *)0x42E68CCCu)     /* 1: chip select by GPIO */
#define spi_no_ctrl    (*(int *)0x42E68CD0u)     /* 1: skip counter/clock programming */
#define spi_max_hz     (*(int *)IMG(4009549c))
#define spi_cs_port    ((u32 *)0x42E68C20u)
#define spi_cs_pin     ((u32 *)0x42E68C2Cu)

#endif
