/* Functions of nonmatching/drivers/pio/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_PIO_H
#define DRIVERS_PIO_H

#include "hw/v3s.h"
#include "rt/bits.h"

void pio_cfg_pin(pio_port_t *port, u32 pin, u32 func, u32 pull);                                 /* 0x400631D4 */
void pio_set_bit(pio_port_t *port, u32 pin);                                                     /* 0x40063274 */
void pio_clr_bit(pio_port_t *port, u32 pin);                                                     /* 0x40063288 */
int pio_get_bit(pio_port_t *port, u32 pin);                                                      /* 0x400632A0 */
void pio_set_drive(pio_port_t *port, u32 pin, u32 drv);                                          /* 0x400632B8 */

#endif
