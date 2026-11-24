/*
 * usb_hid_out_ep_callback -- stock 0x4006D7CC (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * HID OUT report: echo it on EP1 IN.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/hid.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int usb_hid_out_ep_callback(unsigned char *buf, int len) { return usb_device_write_data(1, buf, len); }
