/* Functions of nonmatching/usb/core/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef USB_CORE_H
#define USB_CORE_H

#include "rt/bits.h"

void usb_mode_enter(void);                                                                       /* 0x40069018 */
void usb_mode_exit(void);                                                                        /* 0x40069034 */
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);                      /* 0x4006AA28 */
int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len);                       /* 0x4006AAEC */
void usb_device_send_nullpack_ep0(void);                                                         /* 0x4006AB18 */
void usb_device_read_data_status_ep0(u32 is_complete);                                           /* 0x4006AB24 */
int usb_device_write_data(int ep, unsigned char *databuf, int len);                              /* 0x4006AB30 */
void usb_device_clear_setup_end(void);                                                           /* 0x4006AC40 */
void usb_device_set_address(unsigned char addr);                                                 /* 0x4006AC44 */
void usb_device_set_ep0_state(int state);                                                        /* 0x4006AC5C */
int usb_dev_bsp_init(void);                                                                      /* 0x4006AC68 */
void clear_all_irq(void);                                                                        /* 0x4006ACB8 */
u32 filtrate_irq_misc(u32 irq_misc);                                                             /* 0x4006ACDC */
void usbd_start_work(void);                                                                      /* 0x4006AD0C */
void usbd_stop_work(void);                                                                       /* 0x4006AD2C */
void cfg_udc_command(u32 cmd);                                                                   /* 0x4006AD4C */
void sunxi_udc_disable(void);                                                                    /* 0x4006AD84 */
void usb_handle_ep0(void);                                                                       /* 0x4006ADBC */
void usb_handle_epn_in(int ep);                                                                  /* 0x4006B0E0 */
void usb_handle_epn_out(int ep);                                                                 /* 0x4006B17C */
void usb_irq_handle(void);                                                                       /* 0x4006B290 */
void usb_config_ep_in(int epidx, int maxpack, int type);                                         /* 0x4006B4B8 */
void usb_config_ep_out(int epidx, int maxpack, int type);                                        /* 0x4006B550 */
void usb_device_clear_ep_halt(int epaddr);                                                       /* 0x4006B5E8 */
void sunxi_udc_enable(void);                                                                     /* 0x4006B624 */
int usb_device_init(unsigned char type);                                                         /* 0x4006B6A4 */
void usb_device_exit(void);                                                                      /* 0x4006B728 */
void usb_get_device_descriptor(unsigned char *req);                                              /* 0x4006CFE4 */
void usb_get_cfg_descriptor(unsigned char *req);                                                 /* 0x4006D04C */
void udp_get_dev_descriptor_string(unsigned char *req);                                          /* 0x4006D078 */
void usb_get_descriptor(unsigned char *req);                                                     /* 0x4006D1B0 */
void usb_set_address(u32 addr);                                                                  /* 0x4006D2A0 */
void usb_set_configuration(void);                                                                /* 0x4006D2D8 */
void usb_set_configuration_body(unsigned char *type);                                            /* 0x4006D2DC */
void usb_standard_request(unsigned char *req);                                                   /* 0x4006D378 */
void usb_class_request(unsigned char *req);                                                      /* 0x4006D4B0 */
void usb_setup_handle(unsigned char *req, int len);                                              /* 0x4006D5E8 */

#endif
