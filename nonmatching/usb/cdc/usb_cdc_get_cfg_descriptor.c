/*
 * usb_cdc_get_cfg_descriptor -- stock 0x4006D7DC (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Configuration descriptor, CDC (75 bytes; prints the HID message): 8 or 9 bytes when asked for exactly that, else the whole descriptor.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/cdc.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_cdc_get_cfg_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6), n;
    debug_printf_stub(STR(4008e12c));
    n = len == 8 ? 8 : len == 9 ? 9 : 0x4B;
    usb_device_write_data_ep_pack(0, (unsigned char *)IMG(4008c79c), n);
}
