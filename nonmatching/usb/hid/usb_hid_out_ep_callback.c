/*
 * usb_hid_out_ep_callback -- stock 0x4006D7CC (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * HID OUT report: echo it on EP1 IN.
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);

int usb_device_write_data(int ep, unsigned char *databuf, int len);

int usb_hid_out_ep_callback(unsigned char *buf, int len) { return usb_device_write_data(1, buf, len); }
