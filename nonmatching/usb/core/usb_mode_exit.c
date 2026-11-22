/*
 * usb_mode_exit -- stock 0x40069034 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * usb_device_exit, then disk_remount.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/diskio.h"
#include "usb/core.h"

void usb_mode_exit(void)
{
    usb_device_exit();
    disk_remount();
}
