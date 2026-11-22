/*
 * usb_handle_ep0 -- stock 0x4006ADBC (804 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0 state machine: IDLE reads the 8-byte setup packet, IN continues a long reply from usb_tx_data (the last packet sets usb_tx_pos = 0), OUT reads and prints, END clears usb_xfer_type. FNIRSI sends every setup packet to usb_setup_handle.
 */
#include "hw/usb.h"

int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_setup_handle(unsigned char *req, int len);

void usb_handle_ep0(void)
{
    unsigned char d[8];
    int len, timeout;
    USBC_SelectActiveEp(0);
    if (USBC_Dev_IsEpStall(USBC_EP_TYPE_EP0)) {
        debug_printf_stub(STR(0x4008F9BC));
        USBC_Dev_EpClearStall(USBC_EP_TYPE_EP0);
        usb_ep0_state = 0;
        return;
    }
    if (USBC_Dev_Ctrl_IsSetupEnd()) {
        debug_printf_stub(STR(0x4008E1B8));
        USBC_Dev_Ctrl_ClearSetupEnd();
        usb_ep0_state = 0;
    }
    debug_printf_stub(STR(0x4008EF14), usb_ep0_state);
    debug_printf_stub(STR(0x4008F9CC), USBC_Dev_Read_EP0_CSR());
    if (USBC_Dev_IsReadDataReady(USBC_EP_TYPE_EP0)) {
        usb_ep0_state = 0;
    } else {
        switch ((u32)usb_ep0_state) {
        case 0: break;
        case 1: goto in_phase;
        case 2: goto out_phase;
        case 3: usb_xfer_type = 0; usb_ep0_state = 0; return;
        default: usb_ep0_state = 0; return;
        }
    }
    if (!USBC_Dev_IsReadDataReady(USBC_EP_TYPE_EP0)) {
        debug_printf_stub(STR(0x4008D738));
        return;
    }
    len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
    debug_printf_stub(STR(0x4008F9DC), len);
    if (len != 8) {
        timeout = 16;
        while (timeout-- && len != 8)
            len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
        debug_printf_stub(STR(0x4008EF28), len);
        if (len != 8) {
            debug_printf_stub(STR(0x4008D760), len);
            USBC_Dev_ReadDataStatus(USBC_EP_TYPE_EP0, 0);
            USBC_Dev_EpSendStall(USBC_EP_TYPE_EP0);
            return;
        }
    }
    usb_device_read_data_ep_pack(0, d, 8);
    usb_device_read_data_status_ep0(0);
    usb_ep0_state = (d[0] & 0x80) ? 1 : 2;
    usb_setup_handle(d, 8);
    return;

in_phase:
    if (usb_tx_pos == 0) {
        debug_printf_stub(STR(0x4008E82C));
        usb_ep0_state = 0;
        return;
    }
    debug_printf_stub(STR(0x4008E1D4));
    debug_printf_stub(STR(0x4008EF3C), usb_tx_pos);
    debug_printf_stub(STR(0x4008E844), usb_tx_buf_len);
    if (usb_tx_buf_len - usb_tx_pos > 64) {
        USBC_WritePacket(USBC_SelectFIFO(0), ep_max_len[0], usb_tx_data + usb_tx_pos);
        USBC_Dev_WriteDataStatus(USBC_EP_TYPE_EP0, 0);
        usb_tx_pos += 64;
    } else {
        debug_printf_stub(STR(0x4008FF90));
        USBC_WritePacket(USBC_SelectFIFO(0), usb_tx_buf_len - usb_tx_pos, usb_tx_data + usb_tx_pos);
        USBC_Dev_WriteDataStatus(USBC_EP_TYPE_EP0, 1);
        usb_ep0_state = 0;
        usb_tx_pos = 0;
    }
    return;

out_phase:
    if (!USBC_Dev_IsReadDataReady(USBC_EP_TYPE_EP0)) {
        debug_printf_stub(STR(0x4008D088));
        return;
    }
    len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
    debug_printf_stub(STR(0x4008E85C), len);
    usb_device_read_data_ep_pack(0, d, len);
    usb_device_read_data_status_ep0(0);
    usb_ep0_state = 0;
    debug_printf_stub(STR(0x4008C940), d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
}
