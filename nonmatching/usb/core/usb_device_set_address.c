/*
 * usb_device_set_address -- stock 0x4006AC44 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FADDR and usb_addr.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usb_device_set_address(unsigned char addr)
{
    USBC_Dev_SetAddress(addr);
    usb_addr = addr;
}
