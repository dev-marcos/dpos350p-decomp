/* Functions of nonmatching/drivers/ms5351/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_MS5351_H
#define DRIVERS_MS5351_H

#include "rt/bits.h"

void ms5351_start_clocks(void);                                                                  /* 0x400490B8 */
void ms5351_i2c_start(void);                                                                     /* 0x4005F014 */
void ms5351_i2c_stop(void);                                                                      /* 0x4005F060 */
void ms5351_write_byte(u32 b);                                                                   /* 0x4005F0BC */
void ms5351_write_reg(u32 reg, u32 val);                                                         /* 0x4005F164 */
void ms5351_pins_init(void);                                                                     /* 0x4005F1BC */
void ms5351_set_clk0(u32 freq);                                                                  /* 0x4005F1F0 */
void ms5351_set_clk1(u32 freq);                                                                  /* 0x4005F38C */
void ms5351_write_pll(u32 base, u32 a, u32 b, u32 c);                                            /* 0x4005F55C */
void ms5351_write_ms_int(u32 base, u32 div, u32 rdiv_bits);                                      /* 0x4005F644 */

#endif
