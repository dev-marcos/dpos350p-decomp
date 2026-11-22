/*
 * usb_get_descriptor -- stock 0x4006D1B0 (240 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GET_DESCRIPTOR by type (wValue high byte): 1 device, 2 configuration, 3 string, 0x22 HID report; others are acknowledged with a message.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/hid.h"
#include "usb/mass_storage.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)

void usb_get_descriptor(unsigned char *req)
{
    u32 type = *(unsigned short *)(req + 2) >> 8;
    switch (type) {
    case 1: usb_get_device_descriptor(req); return;
    case 2: usb_get_cfg_descriptor(req); return;
    case 3: udp_get_dev_descriptor_string(req); return;
    case 0x22: usb_hid_get_report_descriptor(); return;
    }
    debug_printf_stub(STR(0x4008D434), type);
    usb_device_read_data_status_ep0(1);
}
