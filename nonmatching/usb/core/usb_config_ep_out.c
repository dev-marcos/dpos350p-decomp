/*
 * usb_config_ep_out -- stock 0x4006B550 (152 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Endpoint epidx != 0: configure it (double FIFO), give it 2*maxpack bytes of FIFO at fifo_offset, enable its interrupt, set ep_max_len (FNIRSI dropped the 4 KiB bound check).
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usb_config_ep_out(int epidx, int maxpack, int type)
{
    u32 old;
    if (!epidx) return;
    debug_printf_stub(STR(4008e244), epidx, maxpack, type);
    old = USBC_GetActiveEp();
    USBC_SelectActiveEp((unsigned char)epidx);
    USBC_Dev_ConfigEp(type, 3, 1, maxpack);
    USBC_ConfigFifo(3, 1, maxpack * 2, fifo_offset);
    fifo_offset += maxpack * 2;
    USBC_INT_EnableEp(3, (unsigned char)epidx);
    USBC_SelectActiveEp((unsigned char)old);
    ep_max_len[epidx] = maxpack;
}
