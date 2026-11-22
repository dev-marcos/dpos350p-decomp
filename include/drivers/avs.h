/* Functions of nonmatching/drivers/avs/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_AVS_H
#define DRIVERS_AVS_H

#include "rt/bits.h"

void avs_init(void);                                                                             /* 0x40042CD8 */
void avs0_set_div_default(void);                                                                 /* 0x40042D40 */
void avs1_set_div_default(void);                                                                 /* 0x40042D4C */
void avs_set_div(u32 idx, u32 div);                                                              /* 0x40042D54 */
void avs0_restart(void);                                                                         /* 0x40042DB8 */
void avs1_restart(void);                                                                         /* 0x40042DE4 */
u32 avs0_read(void);                                                                             /* 0x40042E10 */
u32 avs1_read(void);                                                                             /* 0x40042E2C */

#endif
