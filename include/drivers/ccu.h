/* Functions of nonmatching/drivers/ccu/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_CCU_H
#define DRIVERS_CCU_H

#include "rt/bits.h"

void ccu_set_cpu_pll(u32 f);                                                                     /* 0x400427B0 */
void sys_clock_init(s32 f);                                                                      /* 0x4004282C */
s32 periph0_freq(void);                                                                          /* 0x400428AC */
void ccu_gate_on(s32 code);                                                                      /* 0x40042914 */
void ccu_gate_off(s32 code);                                                                     /* 0x40042978 */

#endif
