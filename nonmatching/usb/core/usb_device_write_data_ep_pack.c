/*
 * usb_device_write_data_ep_pack -- stock 0x4006AA28 (196 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Queue an IN transfer: copy to usb_tx_buf, keep the caller's buffer (FNIRSI) and length, send the first packet; usb_tx_pos = 64 if more follows.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/usbc.h"
#include "util.h"

int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len)
{
    memcpy(usb_tx_buf, databuf, len);
    usb_tx_data = databuf;
    usb_tx_buf_len = len;
    debug_printf_stub(STR(4008eed8), len);
    if (len > ep_max_len[ep]) {
        USBC_WritePacket(USBC_SelectFIFO(ep), ep_max_len[ep], databuf);
        USBC_Dev_WriteDataStatus(ep == 0 ? USBC_EP_TYPE_EP0 : USBC_EP_TYPE_TX, 0);
        usb_tx_pos = 64;
    } else {
        USBC_WritePacket(USBC_SelectFIFO(ep), len, databuf);
        USBC_Dev_WriteDataStatus(ep == 0 ? USBC_EP_TYPE_EP0 : USBC_EP_TYPE_TX, 1);
        usb_tx_pos = 0;
    }
    return 0;
}
