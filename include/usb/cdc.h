/* Functions of nonmatching/usb/cdc/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef USB_CDC_H
#define USB_CDC_H

#include "rt/bits.h"

void usb_cdc_store_line_coding(unsigned char *buf);                                              /* 0x4006CFB0 */
void usb_cdc_get_cfg_descriptor(unsigned char *req);                                             /* 0x4006D7DC */
int usb_cdc_echo(unsigned char *buf, int len);                                                   /* 0x4006D830 */
int usb_cdc_out_ep_callback(unsigned char *buf, int len);                                        /* 0x4006D884 */

#endif
