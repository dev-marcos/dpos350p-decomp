/* Functions of nonmatching/usb/phy/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef USB_PHY_H
#define USB_PHY_H

#include "rt/bits.h"

void usb_phy_nop(void);                                                                          /* 0x4006C108 */
void usb_phy_write(int addr, int data, int len);                                                 /* 0x4006C2B8 */
u32 usb_phy_open_clock_Device(void);                                                             /* 0x4006C384 */
u32 usb_phy_close_clock_Device(void);                                                            /* 0x4006C3D4 */

#endif
