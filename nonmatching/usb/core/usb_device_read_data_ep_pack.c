/*
 * usb_device_read_data_ep_pack -- stock 0x4006AAEC (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Read len bytes from an endpoint FIFO.
 */
#include "hw/usb.h"

int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len)
{
    USBC_ReadPacket(USBC_SelectFIFO(ep), len, databuf);
    return 0;
}
