/*
 * usb_irq_handle -- stock 0x4006B290 (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * USB OTG interrupt (IRQ 103): reset, resume, suspend, disconnect; EP0 (the first EP0 interrupt picks HS or FS packet sizes); RX then TX endpoints 1..3.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usb_irq_handle(void)
{
    u32 old = USBC_GetActiveEp(), misc = USBC_INT_MiscPending();
    u32 tx = USBC_INT_EpPending(USBC_EP_TYPE_TX), rx = USBC_INT_EpPending(USBC_EP_TYPE_RX);
    int i;
    misc = filtrate_irq_misc(misc);
    debug_printf_stub(STR(4008d984));
    debug_printf_stub(STR(4008ce64), misc, tx, rx, 0);
    if (misc & 4) {
        debug_printf_stub(STR(4008ff9c));
        USBC_INT_ClearMiscPending(4);
        clear_all_irq();
        fifo_offset = 0;
        usb_connect = 1;
        usb_ep0_state = 0;
        USBC_SelectActiveEp(0);
        USBC_Dev_SetAddress_default();
        return;
    }
    if (misc & 2) {
        debug_printf_stub(STR(4008f9ec));
        USBC_INT_ClearMiscPending(2);
        usb_connect = 1;
    }
    if (misc & 1) {
        debug_printf_stub(STR(4008f9fc));
        USBC_INT_ClearMiscPending(1);
        usb_connect = 0;
        usb_ep0_state = 0;
    }
    if (misc & 0x20) {
        debug_printf_stub(STR(4008ef78));
        USBC_INT_ClearMiscPending(0x20);
        usb_ep0_state = 0;
        usb_connect = 0;
    }
    if (tx & 1) {
        debug_printf_stub(STR(4008fa0c));
        USBC_INT_ClearEpPending(USBC_EP_TYPE_TX, 0);
        if (current_speed == 0) {
            if (USBC_Dev_QueryTransferMode() == 3) {
                current_speed = 3;
                ep_max_len[1] = 512;
                ep_max_len[2] = 512;
                debug_printf_stub(STR(4008d788));
                debug_printf_stub(STR(4008e88c));
            } else {
                current_speed = 2;
                ep_max_len[1] = 64;
                ep_max_len[2] = 64;
                debug_printf_stub(STR(4008d788));
                debug_printf_stub(STR(4008e8a4));
            }
            debug_printf_stub(STR(4008d788));
        }
        usb_handle_ep0();
    }
    for (i = 1; i < 4; i++)
        if ((rx >> i) & 1) {
            USBC_INT_ClearEpPending(USBC_EP_TYPE_RX, (unsigned char)i);
            usb_handle_epn_out(i);
        }
    for (i = 1; i < 4; i++)
        if ((tx >> i) & 1) {
            USBC_INT_ClearEpPending(USBC_EP_TYPE_TX, (unsigned char)i);
            usb_handle_epn_in(i);
        }
    USBC_SelectActiveEp((unsigned char)old);
}
