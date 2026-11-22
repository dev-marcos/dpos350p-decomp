/*
 * usb_set_configuration_body -- stock 0x4006D2DC (156 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Body of usb_set_configuration with the class-type pointer in r0 (second entry, name OPEN).
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/mass_storage.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)

void usb_set_configuration_body(unsigned char *type)
{
    u32 t = *type;
    u32 ts = t == 4 ? 4 : (t == 1 || t == 2) ? 3 : t == 3 ? 2 : 0;
    if (ts) {
        usb_config_ep_out(1, 512, ts);
        usb_config_ep_in(1, 512, ts);
    }
    usb_device_read_data_status_ep0(1);
    usb_device_clear_setup_end();
    usb_device_send_nullpack_ep0();
    usb_device_set_ep0_state(0);
}
