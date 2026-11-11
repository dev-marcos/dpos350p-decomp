/* Prototypes of stock driver helpers (addresses in symbols/manual.csv). */
#ifndef HW_DRIVERS_H
#define HW_DRIVERS_H
#include "hw/v3s.h"

void pio_cfg_pin(pio_port_t *port, u32 pin, u32 func, u32 pull);   /* 0x400631D4 */
void pio_set_bit(pio_port_t *port, u32 pin);                       /* 0x40063274 */
void pio_clr_bit(pio_port_t *port, u32 pin);                       /* 0x40063288 */
int  pio_get_bit(pio_port_t *port, u32 pin);                       /* 0x400632A0 */
void pio_set_drive(pio_port_t *port, u32 pin, u32 drv);            /* 0x400632B8 */

u32  readl_b(u32 addr);                                            /* 0x40067478 */
void writel_b(u32 addr, u32 v);                                    /* 0x40067480 */

void fpga_bus_input(void);                                         /* 0x400490DC */
void fpga_bus_output(void);                                        /* 0x400490F4 */

extern volatile unsigned char g_fpga_bus_is_output;                /* 0x42E68D18 */

void delay_us(u32 us);                                             /* 0x40042D0C (timer polling) */
void delay_ms(u32 ms);                                             /* 0x40042CF0 (timer polling) */

#define PIO_PG 0x01C208D8u
#define GT_SDA 6   /* PG6 */
#define GT_SCL 7   /* PG7 */
void gt911_i2c_start(void);
void gt911_i2c_stop(void);
void gt911_wait_ack(void);
void gt911_send_ack(void);
void gt911_write_byte(u32 b);
u32  gt911_read_byte(void);
void gt911_write_regs(u32 reg, u32 addr_len, const unsigned char *buf, int len);
void gt911_read_regs(u32 reg, u32 addr_len, unsigned char *buf, int len);
void gt911_ctrl_pins_out(void);
void gt911_int_input(void);
void gt911_reset_seq(void);

/* MS5351M (Si5351-compatible) clock generator on PE20 (SCL) / PE21 (SDA), device 0xC0 */
#define MS_SCL 20
#define MS_SDA 21
void ms5351_i2c_start(void);
void ms5351_i2c_stop(void);
void ms5351_write_byte(u32 b);
void ms5351_write_reg(u32 reg, u32 val);
void ms5351_write_pll(u32 base, u32 a, u32 b, u32 c);
void ms5351_write_ms_int(u32 base, u32 div, u32 rdiv_bits);

#define SYS_INFO_ADDR 0x40544E68u    /* stock SYS_INFO structure (Hack docs/SYS_INFO.md) */
#endif
