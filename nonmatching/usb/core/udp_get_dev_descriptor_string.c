/*
 * udp_get_dev_descriptor_string -- stock 0x4006D078 (312 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * String descriptors 0 (language, max 4 bytes), 1/3, 2 (header rewritten with the length), 4, 5, 6, 0xEE (MS OS string, header rewritten in the 1/3 buffer); max 0x20 or 0x26 bytes; others send nothing.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)

void udp_get_dev_descriptor_string(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6), idx = req[2];
    unsigned char *d;
    debug_printf_stub(STR(0x4008D960), idx);
    switch (idx) {
    case 0:
        usb_device_write_data_ep_pack(0, (unsigned char *)0x400954A0u, len < 5 ? len : 4);
        return;
    case 2:
        if ((int)len >= 0x26) len = 0x26;
        *(unsigned short *)0x40095340u = (unsigned short)(len | 0x300);
        d = (unsigned char *)0x40095340u;
        break;
    case 1:
    case 3: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x40095360u; break;
    case 4: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x40095380u; break;
    case 5: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x400953A0u; break;
    case 6: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x400953C0u; break;
    case 0xEE:
        if (len >= 0x20) len = 0x20;
        *(unsigned short *)0x40095360u = (unsigned short)(len | 0x300);
        d = (unsigned char *)0x40095360u;
        break;
    default:
        usb_device_write_data_ep_pack(0, 0, 0);
        return;
    }
    usb_device_write_data_ep_pack(0, d, len);
}
