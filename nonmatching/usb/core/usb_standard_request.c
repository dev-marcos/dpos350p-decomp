/*
 * usb_standard_request -- stock 0x4006D378 (312 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Standard requests: GET_DESCRIPTOR, SET_CONFIGURATION, SET_ADDRESS, CLEAR_FEATURE (endpoint halt); the others are only reported. Ends with an EP0 status acknowledge.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_standard_request(unsigned char *req)
{
    u32 r = req[1];
    if (r == 6) { debug_printf_stub(STR(4008f92c)); usb_get_descriptor(req); return; }
    if (r == 9) { debug_printf_stub(STR(4008ee60)); usb_set_configuration(); return; }
    if (r == 8) { debug_printf_stub(STR(4008ee74)); debug_printf_stub(STR(4008d020)); }
    else if (r == 0x0B) { debug_printf_stub(STR(4008f93c)); debug_printf_stub(STR(4008d23c)); }
    else if (r == 0x0A) { debug_printf_stub(STR(4008d26c)); }
    else if (r == 5) { debug_printf_stub(STR(4008ff78)); usb_set_address(req[2] & 0x7F); return; }
    else if (r == 7) { debug_printf_stub(STR(4008f94c)); debug_printf_stub(STR(4008d054)); }
    else if (r == 0x0C) { debug_printf_stub(STR(4008ff84)); debug_printf_stub(STR(4008d29c)); }
    else if (r == 1) {
        u32 idx = *(unsigned short *)(req + 4);
        debug_printf_stub(STR(4008f95c));
        usb_device_clear_ep_halt(idx);
    } else {
        debug_printf_stub(STR(4008f96c));
        debug_printf_stub(STR(4008d460), r);
    }
    usb_device_read_data_status_ep0(1);
}
