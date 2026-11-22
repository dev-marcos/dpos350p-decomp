/* Functions of nonmatching/drivers/wdt/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_WDT_H
#define DRIVERS_WDT_H

#include "rt/bits.h"

int wdt_config(u32 interval, u32 cfg, u32 irq);                                                  /* 0x40067488 */
void wdt_enable(void);                                                                           /* 0x4006750C */
int wdt_irq_ack(void);                                                                           /* 0x4006752C */
void wdt_system_reset(void);                                                                     /* 0x40067574 */

#endif
