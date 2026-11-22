/*
 * usb_set_address -- stock 0x4006D2A0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SET_ADDRESS: wait 1 ms, program FADDR, acknowledge, back to EP0 idle.
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);

void delay_ms(u32 ms);
void usb_device_set_address(unsigned char addr);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_device_set_ep0_state(int state);

void usb_set_address(u32 addr)
{
    delay_ms(1);
    usb_device_set_address((unsigned char)addr);
    usb_device_read_data_status_ep0(1);
    debug_printf_stub(STR(0x4008E7CC), addr);
    usb_device_set_ep0_state(0);
}
