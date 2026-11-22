/*
 * usb_get_device_descriptor -- stock 0x4006CFE4 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 18-byte device descriptor of the current class (3 UVC 0x4009543C, 1 HID 0x40095428, 2 CDC 0x40095414, 4 mass storage 0x40095450). Another type leaves the pointer register unset (OPEN: not tested).
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)

void usb_get_device_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6);
    unsigned char *d = 0;
    switch (current_usb_type) {
    case 3: d = (unsigned char *)0x4009543Cu; break;
    case 1: d = (unsigned char *)0x40095428u; break;
    case 2: d = (unsigned char *)0x40095414u; break;
    case 4: d = (unsigned char *)0x40095450u; break;
    }
    debug_printf_stub(STR(0x4008E148));
    usb_device_write_data_ep_pack(0, d, len < 0x12 ? len : 0x12);
}
