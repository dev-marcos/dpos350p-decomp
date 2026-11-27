/*
 * usb_mass_get_cfg_descriptor -- stock 0x4006B980 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Configuration descriptor, mass storage (32 bytes): 8 or 9 bytes when asked for exactly that, else the whole descriptor.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_mass_get_cfg_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6), n;
    debug_printf_stub(STR(4008dc20));
    n = len == 8 ? 8 : len == 9 ? 9 : 0x20;
    usb_device_write_data_ep_pack(0, (unsigned char *)IMG(400953e0), n);
}
