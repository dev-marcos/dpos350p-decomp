/*
 * usb_mode_exit -- stock 0x40069034 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * usb_device_exit, then disk_remount.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void usb_device_exit(void);
void disk_remount(void);

void usb_mode_exit(void)
{
    usb_device_exit();
    disk_remount();
}
