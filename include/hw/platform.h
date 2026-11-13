/* Platform drivers of the stock firmware: GIC/IRQ, UART + debug printf, SPI, SD/MMC, CSI.
 * Addresses are STATIC-CONFIRMED from the stock code; names are DERIVED (see docs/PLATFORM.md). */
#ifndef HW_PLATFORM_H
#define HW_PLATFORM_H
#include "hw/v3s.h"

#define REG16(a) (*(volatile unsigned short *)(a))

void debug_printf_stub(const char *fmt, ...);
void dbg_printf(const char *fmt, ...);
#define STR(a) ((const char *)(a))    /* stock string literal (keeps call arguments identical) */

/* ---- GIC (0x01C80000: distributor +0x1000, CPU interface +0x2000) */
typedef struct {
    u32 base_lo, base_hi;   /* 64-bit base, loaded with LDRD */
    int irq_base;           /* first interrupt id handled */
    int nr;                 /* number of table entries (0x9C) */
} gic_t;
#define g_gic          (*(gic_t *)0x42E68BF0u)
#define irq_table      ((void (**)(void))0x42E67954u)
u32 gic_readl(u32 addr);
void gic_writel(u32 addr, u32 v);
void default_irq_handler(void);
void gic_enable_irq(gic_t *g, int irq);
void gic_disable_irq(gic_t *g, int irq);
int gic_cpu_id(void);
void gic_dist_init(u32 base);
void gic_cpu_init(u32 base);
void local_irq_enable(void);

/* ---- UART / console */
#define g_uart_port    (*(int *)0x42E68CDCu)
u32 readl_g(u32 addr);
void writel_g(u32 addr, u32 v);
void uart_putc(int port, int c);
void console_putc(int c);
void console_puts(const char *s);
void console_putn(int c, int n);
void console_put_reversed(const char *buf, int n);
void console_print_num(int val, int base, int width, int pad);
const char *console_format_conv(const char *p, int arg);
unsigned int strlen(const char *s);

/* ---- SPI (controller n at 0x01C68000 + n * 0x1000) */
#define SPI_BASE(n)    (0x01C68000u + ((u32)(n) << 12))
#define spi_dual_mode  (*(int *)0x42E68CC4u)     /* BCC bits 31:28 */
#define spi_cs_index   (*(int *)0x42E68CC8u)
#define spi_soft_cs    (*(int *)0x42E68CCCu)     /* 1: chip select by GPIO */
#define spi_no_ctrl    (*(int *)0x42E68CD0u)     /* 1: skip counter/clock programming */
#define spi_max_hz     (*(int *)0x4009549Cu)
#define spi_cs_port    ((u32 *)0x42E68C20u)
#define spi_cs_pin     ((u32 *)0x42E68C2Cu)
u32 readb_f(u32 addr);
u32 readl_f(u32 addr);
void writeb_f(u32 addr, u32 v);
void writel_f(u32 addr, u32 v);
s32 spi0_clk_init(int ctrl);
void pio_set_bit(void *port, u32 pin);
void pio_clr_bit(void *port, u32 pin);
void pio_cfg_pin(void *port, u32 pin, u32 func, u32 pull);
void pio_set_drive(void *port, u32 pin, u32 level);
void sys_spi_set_counts(int spi, u32 txn, u32 total);
void sys_spi_start_xch(int spi);
int sys_spi_cpu_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len);
int sys_spi_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len);

#endif
