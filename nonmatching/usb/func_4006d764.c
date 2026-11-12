/*
 * usb_hid_get_report_descriptor -- stock 0x4006D764 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 35-byte HID report descriptor at 0x4008D9A8.
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);

int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);

void usb_hid_get_report_descriptor(void) { usb_device_write_data_ep_pack(0, (unsigned char *)0x4008D9A8u, 0x23); }
