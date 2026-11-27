/*
 * usb_handle_epn_in -- stock 0x4006B0E0 (156 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TX endpoint interrupt: clear a stall; when the FIFO is free and the class is mass storage (4), run its IN callback (the HID callback of EasyUSB is gone).
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "usb/usbc.h"

void usb_handle_epn_in(int ep)
{
    u32 old;
    debug_printf_stub(STR(4008ef50), ep);
    old = USBC_GetActiveEp();
    USBC_SelectActiveEp((unsigned char)ep);
    if (USBC_Dev_IsEpStall(USBC_EP_TYPE_TX)) {
        debug_printf_stub(STR(4008e1f0), ep);
        USBC_Dev_EpClearStall(USBC_EP_TYPE_TX);
    }
    if (!USBC_Dev_IsWriteDataReady(USBC_EP_TYPE_TX)) {
        debug_printf_stub(STR(4008e874), ep);
        if (current_usb_type == 4)
            usb_mass_in_ep_callback();
    }
    USBC_SelectActiveEp((unsigned char)old);
}
