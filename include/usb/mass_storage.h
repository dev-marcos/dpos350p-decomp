/* Functions of nonmatching/usb/mass_storage/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef USB_MASS_STORAGE_H
#define USB_MASS_STORAGE_H

#include "rt/bits.h"

void usb_disk_hook(void);                                                                        /* 0x4006A734 */
u32 usb_disk_sd_read(u32 a, u32 lba, u32 count, u32 d);                                          /* 0x4006A738 */
u32 usb_disk_sd_write(u32 a, u32 lba, u32 count, u32 d);                                         /* 0x4006A868 */
u32 usb_mass_disk_read(u32 a, u32 b, u32 c);                                                     /* 0x4006A964 */
u32 usb_mass_disk_write(u32 a, u32 b, u32 c);                                                    /* 0x4006A978 */
void usb_disk_geometry_update(void);                                                             /* 0x4006A98C */
int usb_mass_write(int unused, unsigned char *buf, int len);                                     /* 0x4006B95C */
unsigned char *usb_mass_max_lun_ptr(void);                                                       /* 0x4006B978 */
void usb_mass_get_cfg_descriptor(unsigned char *req);                                            /* 0x4006B980 */
int usb_mass_in_ep_callback(void);                                                               /* 0x4006B9D8 */
void usb_mass_out_ep_callback(unsigned char *buf, u32 len);                                      /* 0x4006B9F0 */
void usb_mass_in_handler(void);                                                                  /* 0x4006B9F4 */
void usb_mass_out_handler(unsigned char *buf, u32 len);                                          /* 0x4006BA98 */
void usb_mass_send_data(unsigned char *buf, int len);                                            /* 0x4006C04C */
void usb_mass_send_csw(u32 status, u32 next);                                                    /* 0x4006C08C */
int scsi_lun_invalid(u32 lun);                                                                   /* 0x4006D888 */
void scsi_inquiry(u32 lun);                                                                      /* 0x4006D898 */
void scsi_read_format_capacities(u32 lun);                                                       /* 0x4006D8CC */
void scsi_read_capacity10(u32 lun);                                                              /* 0x4006D95C */
void scsi_mode_sense6(u32 lun);                                                                  /* 0x4006D9FC */
void scsi_mode_sense10(u32 lun);                                                                 /* 0x4006DA08 */
void scsi_request_sense(u32 lun);                                                                /* 0x4006DA14 */
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);                                              /* 0x4006DA2C */
void scsi_start_stop(u32 lun);                                                                   /* 0x4006DA3C */
void scsi_read10(u32 lun, u32 lba, u32 count);                                                   /* 0x4006DA48 */
void scsi_write10(u32 lun, u32 lba, u32 count);                                                  /* 0x4006DB0C */
void scsi_verify10(u32 lun);                                                                     /* 0x4006DBA4 */
void scsi_test_unit_ready(u32 lun);                                                              /* 0x4006DBF0 */
void scsi_format_unit(u32 lun);                                                                  /* 0x4006DC44 */
void scsi_invalid_cmd(u32 lun);                                                                  /* 0x4006DC90 */
int scsi_check_range(u32 lun, u32 op, u32 lba, u32 count);                                       /* 0x4006DCE4 */

#endif
