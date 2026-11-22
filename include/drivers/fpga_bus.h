/* Functions of nonmatching/drivers/fpga_bus/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_FPGA_BUS_H
#define DRIVERS_FPGA_BUS_H

#include "rt/bits.h"

void fpga_bus_input(void);                                                                       /* 0x400490DC */
void fpga_bus_output(void);                                                                      /* 0x400490F4 */
void fpga_bus_init(void);                                                                        /* 0x40049110 */
u32 fpga_data_byte_read(void);                                                                   /* 0x400491B8 */
void fpga_data_byte_write(u32 v);                                                                /* 0x40049214 */
void fpga_selector_command_write(u32 v);                                                         /* 0x4004927C */

#endif
