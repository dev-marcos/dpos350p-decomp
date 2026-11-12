/*
 * usbd_stop_work -- stock 0x4006AD2C (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Soft connect off.
 */
#include "hw/usb.h"

void usbd_stop_work(void)
{
    debug_printf_stub(STR(0x4008F9AC));
    USBC_Dev_ConectSwitch(0);
}
