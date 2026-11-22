/*
 * usbd_start_work -- stock 0x4006AD0C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Soft connect on.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usbd_start_work(void)
{
    debug_printf_stub(STR(0x4008EEEC));
    USBC_Dev_ConectSwitch(1);
}
