/* Functions of nonmatching/drivers/lradc/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_LRADC_H
#define DRIVERS_LRADC_H

#include "rt/bits.h"

u32 lradc_data_read(void);                                                                       /* 0x40068E74 */
void lradc_init(void);                                                                           /* 0x40068E94 */

#endif
