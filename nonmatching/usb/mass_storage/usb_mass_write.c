/*
 * usb_mass_write -- stock 0x4006B95C (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * usb_device_write_data(1, buf, len) ignoring its first argument; returns 0.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int usb_mass_write(int unused, unsigned char *buf, int len)
{
    (void)unused;
    usb_device_write_data(1, buf, len);
    return 0;
}
