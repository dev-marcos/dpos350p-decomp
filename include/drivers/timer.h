/* Functions of nonmatching/drivers/timer/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include "rt/bits.h"

void timer_setup(u32 n, u32 interval, u32 irq);                                                  /* 0x40042B60 */
void timer_start(u32 n);                                                                         /* 0x40042C1C */
void timer_stop(u32 n);                                                                          /* 0x40042C40 */
void timer0_isr(void);                                                                           /* 0x40042C64 */
void timer0_init(void);                                                                          /* 0x40042C94 */
void delay_ms(u32 n);                                                                            /* 0x40042CF0 */
void delay_us(u32 n);                                                                            /* 0x40042D0C */
void timers_init(void);                                                                          /* 0x40042D28 */

#endif
