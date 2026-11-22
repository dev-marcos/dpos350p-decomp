/* Functions of nonmatching/drivers/gt911/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_GT911_H
#define DRIVERS_GT911_H

#include "rt/bits.h"

void gt911_i2c_start(void);                                                                      /* 0x40049350 */
void gt911_i2c_stop(void);                                                                       /* 0x40049428 */
void gt911_wait_ack(void);                                                                       /* 0x40049490 */
void gt911_send_ack(void);                                                                       /* 0x4004951C */
void gt911_write_byte(u32 b);                                                                    /* 0x40049554 */
u32 gt911_read_byte(void);                                                                       /* 0x400495CC */
void gt911_write_regs(u32 reg, u32 addr_len, const unsigned char *buf, int len);                 /* 0x40049688 */
void gt911_read_regs(u32 reg, u32 addr_len, unsigned char *buf, int len);                        /* 0x40049700 */
void gt911_i2c_pins_init(void);                                                                  /* 0x40049788 */
void gt911_ctrl_pins_out(void);                                                                  /* 0x400497D4 */
void gt911_int_input(void);                                                                      /* 0x40049820 */
void gt911_reset_seq(void);                                                                      /* 0x40049834 */
void gt911_bus_init(void);                                                                       /* 0x400498A4 */
void gt911_write16(u32 reg, const unsigned char *buf, int len);                                  /* 0x400498C4 */
void gt911_read16(u32 reg, unsigned char *buf, int len);                                         /* 0x400498D4 */
void gt911_poll(void);                                                                           /* 0x400498E4 */
void gt911_pins_init(void);                                                                      /* 0x40049A5C */

#endif
