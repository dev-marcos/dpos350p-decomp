/*
 * usb_hid_get_cfg_descriptor -- stock 0x4006D774 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Configuration descriptor, HID (41 bytes): 8 or 9 bytes when asked for exactly that, else the whole descriptor.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/hid.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_hid_get_cfg_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6), n;
    debug_printf_stub(STR(4008e12c));
    n = len == 8 ? 8 : len == 9 ? 9 : 0x29;
    usb_device_write_data_ep_pack(0, (unsigned char *)IMG(4008d48c), n);
}
