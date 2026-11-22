/*
 * usb_class_request -- stock 0x4006D4B0 (312 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Class requests: HID SET_REPORT/SET_IDLE (message only), CDC SET_LINE_CODING (waits 200 us for the 7-byte data stage), GET_LINE_CODING, SET_CONTROL_LINE_STATE, mass storage GET_MAX_LUN (0xFE).
 */
#include "hw/usb.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);
#define max_lun_rsp (*(unsigned char *)0x42E68D1Eu)

void delay_us(u32 us);
int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len);
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_device_set_ep0_state(int state);
void usb_device_clear_setup_end(void);
void usb_cdc_store_line_coding(unsigned char *buf);
unsigned char *usb_mass_max_lun_ptr(void);

void usb_class_request(unsigned char *req)
{
    unsigned char buf[8];
    u32 r = req[1], len;
    if (r == 9) { debug_printf_stub(STR(0x4008CF10)); return; }
    if (r == 0x0A) { debug_printf_stub(STR(0x4008CF48)); return; }
    if (r == 0x20) {
        debug_printf_stub(STR(0x4008EE88));
        line_coding_rx = 1;
        delay_us(200);
        len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
        debug_printf_stub(STR(0x4008E164), len);
        if (len != 7) return;
        usb_device_read_data_ep_pack(0, buf, 7);
        usb_cdc_store_line_coding(buf);
        usb_device_read_data_status_ep0(1);
        usb_device_set_ep0_state(0);
        line_coding_rx = 0;
        return;
    }
    if (r == 0x21) {
        usb_device_write_data_ep_pack(0, line_coding, 7);
        debug_printf_stub(STR(0x4008EE9C));
        return;
    }
    if (r == 0x22) {
        debug_printf_stub(STR(0x4008E180));
        usb_device_clear_setup_end();
        usb_device_set_ep0_state(0);
        return;
    }
    if (r == 0xFE) {
        max_lun_rsp = *usb_mass_max_lun_ptr();
        debug_printf_stub(STR(0x4008E7E4), max_lun_rsp);
        usb_device_write_data_ep_pack(0, &max_lun_rsp, *(unsigned short *)(req + 6));
        return;
    }
    debug_printf_stub(STR(0x4008CE28), r);
}
