/*
 * usb_set_address -- stock 0x4006D2A0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SET_ADDRESS: wait 1 ms, program FADDR, acknowledge, back to EP0 idle.
 */
#include "hw/usb.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_set_address(u32 addr)
{
    delay_ms(1);
    usb_device_set_address((unsigned char)addr);
    usb_device_read_data_status_ep0(1);
    debug_printf_stub(STR(0x4008E7CC), addr);
    usb_device_set_ep0_state(0);
}
