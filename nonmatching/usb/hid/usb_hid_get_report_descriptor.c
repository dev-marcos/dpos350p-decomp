/*
 * usb_hid_get_report_descriptor -- stock 0x4006D764 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 35-byte HID report descriptor at 0x4008D9A8.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/hid.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_hid_get_report_descriptor(void) { usb_device_write_data_ep_pack(0, (unsigned char *)0x4008D9A8u, 0x23); }
