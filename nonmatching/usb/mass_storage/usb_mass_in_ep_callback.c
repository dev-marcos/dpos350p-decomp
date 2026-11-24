/*
 * usb_mass_in_ep_callback -- stock 0x4006B9D8 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP1 IN done: run the IN state machine; returns 0.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int usb_mass_in_ep_callback(void)
{
    usb_mass_in_handler();
    return 0;
}
