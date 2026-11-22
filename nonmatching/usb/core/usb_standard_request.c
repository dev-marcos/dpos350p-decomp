/*
 * usb_standard_request -- stock 0x4006D378 (312 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Standard requests: GET_DESCRIPTOR, SET_CONFIGURATION, SET_ADDRESS, CLEAR_FEATURE (endpoint halt); the others are only reported. Ends with an EP0 status acknowledge.
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);

void usb_get_descriptor(unsigned char *req);
void usb_set_configuration(void);
void usb_set_address(u32 addr);
void usb_device_clear_ep_halt(int epaddr);
void usb_device_read_data_status_ep0(u32 is_complete);

void usb_standard_request(unsigned char *req)
{
    u32 r = req[1];
    if (r == 6) { debug_printf_stub(STR(0x4008F92C)); usb_get_descriptor(req); return; }
    if (r == 9) { debug_printf_stub(STR(0x4008EE60)); usb_set_configuration(); return; }
    if (r == 8) { debug_printf_stub(STR(0x4008EE74)); debug_printf_stub(STR(0x4008D020)); }
    else if (r == 0x0B) { debug_printf_stub(STR(0x4008F93C)); debug_printf_stub(STR(0x4008D23C)); }
    else if (r == 0x0A) { debug_printf_stub(STR(0x4008D26C)); }
    else if (r == 5) { debug_printf_stub(STR(0x4008FF78)); usb_set_address(req[2] & 0x7F); return; }
    else if (r == 7) { debug_printf_stub(STR(0x4008F94C)); debug_printf_stub(STR(0x4008D054)); }
    else if (r == 0x0C) { debug_printf_stub(STR(0x4008FF84)); debug_printf_stub(STR(0x4008D29C)); }
    else if (r == 1) {
        u32 idx = *(unsigned short *)(req + 4);
        debug_printf_stub(STR(0x4008F95C));
        usb_device_clear_ep_halt(idx);
    } else {
        debug_printf_stub(STR(0x4008F96C));
        debug_printf_stub(STR(0x4008D460), r);
    }
    usb_device_read_data_status_ep0(1);
}
