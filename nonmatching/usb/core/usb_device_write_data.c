/*
 * usb_device_write_data -- stock 0x4006AB30 (272 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Bulk IN: ep_max_len-sized packets, up to 10 000 000 polls for each FIFO slot; -1 on timeout or when not connected.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/usbc.h"

int usb_device_write_data(int ep, unsigned char *databuf, int len)
{
    volatile int Timeout = 10000000;
    int pack_len = ep_max_len[ep], write_len, data_pos = 0, length = len;
    u32 old;
    void *fifo;
    if (!usb_connect) {
        debug_printf_stub(STR(0x4008D2CC));
        return -1;
    }
    debug_printf_stub(STR(0x4008DBA0), len);
    old = USBC_GetActiveEp();
    fifo = USBC_SelectFIFO(ep);
    USBC_SelectActiveEp((unsigned char)ep);
    do {
        while (USBC_Dev_IsWriteDataReady(USBC_EP_TYPE_TX) && --Timeout)
            ;
        write_len = pack_len > length ? length : pack_len;
        USBC_WritePacket(fifo, write_len, databuf + data_pos);
        USBC_Dev_WriteDataStatus(USBC_EP_TYPE_TX, 1);
        data_pos += write_len;
        length -= write_len;
        if (Timeout == 0) {
            USBC_SelectActiveEp((unsigned char)old);
            debug_printf_stub(STR(0x4008DBC0));
            return -1;
        }
        Timeout = 10000000;
    } while (length > 0);
    USBC_SelectActiveEp((unsigned char)old);
    return 0;
}
