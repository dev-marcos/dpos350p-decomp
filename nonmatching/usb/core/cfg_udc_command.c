/*
 * cfg_udc_command -- stock 0x4006AD4C (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 1 = start, 2 = stop, 3 = 'reset not supported', else 'unknown cmd'.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"

void cfg_udc_command(u32 cmd)
{
    switch (cmd) {
    case 1: usbd_start_work(); break;
    case 2: usbd_stop_work(); break;
    case 3: debug_printf_stub(STR(4008e19c)); break;
    default: debug_printf_stub(STR(4008e814), cmd); break;
    }
}
