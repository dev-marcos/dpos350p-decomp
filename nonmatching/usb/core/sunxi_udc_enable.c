/*
 * sunxi_udc_enable -- stock 0x4006B624 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Speed unknown, bulk/HS transfer mode, misc interrupt enables (FNIRSI passes the bit numbers 0, 1, 2, 5 as masks), EP0 interrupt, soft connect.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/usbc.h"

void sunxi_udc_enable(void)
{
    debug_printf_stub(STR(4008e260));
    current_speed = 0;
    debug_printf_stub(STR(4008d2fc));
    USBC_Dev_ConfigTransferMode(4, 3);
    USBC_INT_EnableUsbMiscUint(0);
    USBC_INT_EnableUsbMiscUint(1);
    USBC_INT_EnableUsbMiscUint(2);
    USBC_INT_EnableUsbMiscUint(5);
    USBC_INT_EnableEp(USBC_EP_TYPE_TX, 0);
    cfg_udc_command(1);
}
