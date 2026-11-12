/*
 * usb_get_cfg_descriptor -- stock 0x4006D04C (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Configuration descriptor of the current class.
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);

void usb_hid_get_cfg_descriptor(unsigned char *req);
void usb_cdc_get_cfg_descriptor(unsigned char *req);
int usb_uvc_get_cfg_descriptor(unsigned char *req);
void usb_mass_get_cfg_descriptor(unsigned char *req);

void usb_get_cfg_descriptor(unsigned char *req)
{
    switch (current_usb_type) {
    case 1: usb_hid_get_cfg_descriptor(req); break;
    case 2: usb_cdc_get_cfg_descriptor(req); break;
    case 3: usb_uvc_get_cfg_descriptor(req); break;
    case 4: usb_mass_get_cfg_descriptor(req); break;
    }
}
