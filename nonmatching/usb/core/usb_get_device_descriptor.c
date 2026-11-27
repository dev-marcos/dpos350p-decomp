/*
 * usb_get_device_descriptor -- stock 0x4006CFE4 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 18-byte device descriptor of the current class (3 UVC 0x4009543C, 1 HID 0x40095428, 2 CDC 0x40095414, 4 mass storage 0x40095450). Another type leaves the pointer register unset (OPEN: not tested).
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_get_device_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6);
    unsigned char *d = 0;
    switch (current_usb_type) {
    case 3: d = (unsigned char *)IMG(4009543c); break;
    case 1: d = (unsigned char *)IMG(40095428); break;
    case 2: d = (unsigned char *)IMG(40095414); break;
    case 4: d = (unsigned char *)IMG(40095450); break;
    }
    debug_printf_stub(STR(4008e148));
    usb_device_write_data_ep_pack(0, d, len < 0x12 ? len : 0x12);
}
