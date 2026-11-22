/*
 * usb_device_clear_ep_halt -- stock 0x4006B5E8 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clear HALT on the TX (bit 7 set) or RX endpoint.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usb_device_clear_ep_halt(int epaddr)
{
    u32 old = USBC_GetActiveEp();
    USBC_SelectActiveEp(epaddr & 0x7F);
    USBC_Clear_Feature_HALT((epaddr & 0x80) ? USBC_EP_TYPE_TX : USBC_EP_TYPE_RX);
    USBC_SelectActiveEp((unsigned char)old);
}
