/*
 * usb_get_cfg_descriptor -- stock 0x4006D04C (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Configuration descriptor of the current class.
 */
#include "hw/usb.h"
#include "usb/cdc.h"
#include "usb/core.h"
#include "usb/hid.h"
#include "usb/mass_storage.h"
#include "usb/uvc.h"
#include "shared/usb.h"

void usb_get_cfg_descriptor(unsigned char *req)
{
    switch (current_usb_type) {
    case 1: usb_hid_get_cfg_descriptor(req); break;
    case 2: usb_cdc_get_cfg_descriptor(req); break;
    case 3: usb_uvc_get_cfg_descriptor(req); break;
    case 4: usb_mass_get_cfg_descriptor(req); break;
    }
}
