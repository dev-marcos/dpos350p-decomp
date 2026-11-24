/*
 * usb_setup_handle -- stock 0x4006D5E8 (184 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy and print the 8-byte setup packet, then dispatch standard (type 0) or class (type 1) requests; vendor requests are ignored.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "util.h"
#include "shared/usb.h"

void usb_setup_handle(unsigned char *req, int len)
{
    volatile int Timeout = 10000000;
    (void)Timeout;
    if (len != 8) {
        debug_printf_stub(STR(0x4008EEC4));
        return;
    }
    memcpy(setup_req, req, 8);
    debug_printf_stub(STR(0x4008E7FC), setup_req[0]);
    debug_printf_stub(STR(0x4008EEB0), setup_req[1]);
    debug_printf_stub(STR(0x4008F97C), *(unsigned short *)(setup_req + 2));
    debug_printf_stub(STR(0x4008F98C), *(unsigned short *)(setup_req + 4));
    debug_printf_stub(STR(0x4008F99C), *(unsigned short *)(setup_req + 6));
    if (!(setup_req[0] & 0x60))
        usb_standard_request(setup_req);
    else if ((setup_req[0] & 0x60) == 0x20)
        usb_class_request(setup_req);
}
