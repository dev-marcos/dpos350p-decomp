/* Functions of nonmatching/power/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef POWER_H
#define POWER_H

#include "rt/bits.h"

void power_monitor_init(void);                                                                   /* 0x4004A198 */
void auto_off_arm(void);                                                                         /* 0x4004A1D0 */
void auto_off_check(void);                                                                       /* 0x4004A1F4 */
void charge_status_read(void);                                                                   /* 0x4004A278 */
void battery_coeffs_default(void);                                                               /* 0x4004A2A0 */
void battery_level_update(void);                                                                 /* 0x4004A2D0 */
void battery_empty_check(void);                                                                  /* 0x4004A3C0 */
void battery_service(void);                                                                      /* 0x4004A448 */

#endif
