/*
 * usb_handle_epn_out -- stock 0x4006B17C (276 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * RX endpoint interrupt: read the packet into usb_rx_buf and pass it to the class (1 HID, 2 CDC, 3 none, 4 mass storage).
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/cdc.h"
#include "usb/core.h"
#include "usb/hid.h"
#include "usb/mass_storage.h"
#include "usb/usbc.h"

void usb_handle_epn_out(int ep)
{
    u32 old, len;
    debug_printf_stub(STR(0x4008EF64), ep);
    old = USBC_GetActiveEp();
    USBC_SelectActiveEp((unsigned char)ep);
    if (USBC_Dev_IsEpStall(USBC_EP_TYPE_RX)) {
        debug_printf_stub(STR(0x4008E20C), ep);
        USBC_Dev_EpClearStall(USBC_EP_TYPE_RX);
    }
    if (USBC_Dev_IsReadDataReady(USBC_EP_TYPE_RX)) {
        len = USBC_ReadLenFromFifo(USBC_EP_TYPE_RX);
        debug_printf_stub(STR(0x4008DBE0), ep, len);
        usb_device_read_data_ep_pack(ep, usb_rx_buf, len);
        usb_rx_buf_len = len;
        switch (current_usb_type) {
        case 1: usb_hid_out_ep_callback(usb_rx_buf, len); break;
        case 2: usb_cdc_out_ep_callback(usb_rx_buf, len); break;
        case 3: break;
        case 4: usb_mass_out_ep_callback(usb_rx_buf, len); break;
        }
        USBC_Dev_ReadDataStatus(USBC_EP_TYPE_RX, 1);
    }
    USBC_SelectActiveEp((unsigned char)old);
}
