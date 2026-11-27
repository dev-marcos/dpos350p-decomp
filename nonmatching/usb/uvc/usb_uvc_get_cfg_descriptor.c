/*
 * usb_uvc_get_cfg_descriptor -- stock 0x4006DDC0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * UVC configuration descriptor (0x400950B4), wLength bytes.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "usb/uvc.h"
#include "shared/usb.h"

int usb_uvc_get_cfg_descriptor(unsigned char *req)
{
    debug_printf_stub(STR(4008e2b4));
    return usb_device_write_data_ep_pack(0, (unsigned char *)IMG(400950b4), *(unsigned short *)(req + 6));
}
