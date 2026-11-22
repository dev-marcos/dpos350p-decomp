/*
 * usb_mode_enter -- stock 0x40069018 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Updates the USB disk geometry, then usb_device_init(4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "usb/core.h"
#include "usb/mass_storage.h"

void usb_mode_enter(void)
{
    usb_disk_geometry_update();
    usb_device_init(4);
}
