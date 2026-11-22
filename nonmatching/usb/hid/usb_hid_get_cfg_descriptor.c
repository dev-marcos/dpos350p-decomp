/*
 * usb_hid_get_cfg_descriptor -- stock 0x4006D774 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Configuration descriptor, HID (41 bytes): 8 or 9 bytes when asked for exactly that, else the whole descriptor.
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);

int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);

void usb_hid_get_cfg_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6), n;
    debug_printf_stub(STR(0x4008E12C));
    n = len == 8 ? 8 : len == 9 ? 9 : 0x29;
    usb_device_write_data_ep_pack(0, (unsigned char *)0x4008D48Cu, n);
}
