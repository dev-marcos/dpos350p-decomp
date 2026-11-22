/* Functions of nonmatching/usb/hid/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef USB_HID_H
#define USB_HID_H

#include "rt/bits.h"

void usb_hid_get_report_descriptor(void);                                                        /* 0x4006D764 */
void usb_hid_get_cfg_descriptor(unsigned char *req);                                             /* 0x4006D774 */
int usb_hid_out_ep_callback(unsigned char *buf, int len);                                        /* 0x4006D7CC */

#endif
