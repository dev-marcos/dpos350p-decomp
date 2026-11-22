/*
 * usb_device_exit -- stock 0x4006B728 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Disable, close the clocks, reset the stack state.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/phy.h"

void usb_device_exit(void)
{
    dbg_printf(STR(0x4008EFB4));
    sunxi_udc_disable();
    usb_phy_close_clock_Device();
    current_usb_type = 0;
    current_speed = 0;
    usb_connect = 0;
    usb_ep0_state = 0;
    usb_addr = 0;
    usb_xfer_type = 0;
}
