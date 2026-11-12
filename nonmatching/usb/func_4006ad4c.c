/*
 * cfg_udc_command -- stock 0x4006AD4C (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 1 = start, 2 = stop, 3 = 'reset not supported', else 'unknown cmd'.
 */
#include "hw/usb.h"

void usbd_start_work(void);
void usbd_stop_work(void);

void cfg_udc_command(u32 cmd)
{
    switch (cmd) {
    case 1: usbd_start_work(); break;
    case 2: usbd_stop_work(); break;
    case 3: debug_printf_stub(STR(0x4008E19C)); break;
    default: debug_printf_stub(STR(0x4008E814), cmd); break;
    }
}
