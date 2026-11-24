/*
 * usb_cdc_store_line_coding -- stock 0x4006CFB0 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy 7 bytes of SET_LINE_CODING data to line_coding; clear the 'waiting for data' flag.
 */
#include "hw/usb.h"
#include "usb/cdc.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_cdc_store_line_coding(unsigned char *buf)
{
    line_coding_rx = 0;
    *(u32 *)line_coding = *(u32 *)buf;
    line_coding[4] = buf[4];
    line_coding[5] = buf[5];
    line_coding[6] = buf[6];
}
