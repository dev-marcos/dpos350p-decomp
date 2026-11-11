/* Allwinner V3s/R11 peripheral addresses and the PIO port layout used by the stock drivers.
 * Register names follow the V3s user manual; their use by the stock code is DERIVED. */
#ifndef HW_V3S_H
#define HW_V3S_H
#include "rt/bits.h"

#define REG32(a) (*(volatile u32 *)(a))
#define REG8(a)  (*(volatile unsigned char *)(a))

/* PIO: one port block every 0x24 bytes from 0x01C20800 (A=0, B=1, C=2, ... G=6) */
typedef struct {
    volatile u32 cfg[4];   /* +0x00 function select, 4 bits per pin */
    volatile u32 dat;      /* +0x10 data */
    volatile u32 drv[2];   /* +0x14 drive strength, 2 bits per pin */
    volatile u32 pul[2];   /* +0x1C pull up/down, 2 bits per pin */
} pio_port_t;

#define PIO_BASE   0x01C20800u
#define PIO_PORT(n) ((pio_port_t *)(PIO_BASE + 0x24u * (n)))
#define PIO_PC     0x01C20848u
#define PIO_PE     0x01C20890u
#define PIO_PF     0x01C208B4u

/* Watchdog 0 (timer block 0x01C20C00) */
#define WDOG0_IRQ_EN  0x01C20CA0u
#define WDOG0_IRQ_STA 0x01C20CA4u
#define WDOG0_CFG     0x01C20CB4u
#define WDOG0_MODE    0x01C20CB8u

#endif
