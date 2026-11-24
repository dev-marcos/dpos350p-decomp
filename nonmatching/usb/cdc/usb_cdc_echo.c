/*
 * usb_cdc_echo -- stock 0x4006D830 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CDC: echo the data on EP1 IN; a multiple of 512 bytes is followed by a zero-length packet.
 */
#include "hw/usb.h"
#include "usb/cdc.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int usb_cdc_echo(unsigned char *buf, int len)
{
    int r = usb_device_write_data(1, buf, len);
    if (r == 0 && (len & 0x1FF) == 0)
        return usb_device_write_data(1, buf, 0);
    return r;
}
