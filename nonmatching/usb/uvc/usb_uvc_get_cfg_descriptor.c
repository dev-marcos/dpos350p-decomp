/*
 * usb_uvc_get_cfg_descriptor -- stock 0x4006DDC0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * UVC configuration descriptor (0x400950B4), wLength bytes.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "usb/uvc.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)

int usb_uvc_get_cfg_descriptor(unsigned char *req)
{
    debug_printf_stub(STR(0x4008E2B4));
    return usb_device_write_data_ep_pack(0, (unsigned char *)0x400950B4u, *(unsigned short *)(req + 6));
}
