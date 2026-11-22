/*
 * usb_mass_send_csw -- stock 0x4006C08C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Send the 13-byte CSW with this status; next stage 4 (next != 0) or 5.
 */
#include "hw/usb.h"

#define cbw            ((unsigned char *)0x42E68B90u)   /* 31-byte command block wrapper */
#define CBW_SIG        (*(u32 *)0x42E68B90u)
#define CBW_DLEN       (*(u32 *)0x42E68B98u)
#define CBW_FLAGS      cbw[0x0C]
#define CBW_LUN        cbw[0x0D]
#define CBW_CBLEN      cbw[0x0E]
#define CBW_OP         cbw[0x0F]
#define csw            ((unsigned char *)0x42E68C10u)   /* 13-byte command status wrapper */
#define CSW_SIG        (*(u32 *)0x42E68C10u)
#define CSW_TAG        (*(u32 *)0x42E68C14u)
#define CSW_RESIDUE    (*(u32 *)0x42E68C18u)
#define mass_rx_ptr    (*(unsigned char **)0x42E68CE0u)
#define mass_lba       (*(u32 *)0x42E68CE4u)
#define mass_count     (*(u32 *)0x42E68CE8u)
#define lun_blksize    ((u32 *)0x42E68D0Cu)
#define lun_blocks     ((u32 *)0x42E68D10u)
#define mass_rx_len    (*(unsigned short *)0x42E68D16u)
#define mass_flags     (*(unsigned char *)0x42E68D1Cu)
#define mass_stage     (*(unsigned char *)0x42E68D1Du)  /* 0 CBW, 1 data out, 2 data in, 3 sent, 4/5 CSW */
#define max_lun_rsp    (*(unsigned char *)0x42E68D1Eu)
#define max_lun        (*(volatile u32 *)0x42E68D23u)   /* read as an unaligned word by the CBW check */
#define sense_data     ((unsigned char *)0x40095400u)   /* 18-byte fixed-format sense */
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);
int usb_mass_write(int unused, unsigned char *buf, int len);
int scsi_lun_invalid(u32 lun);
int scsi_check_range(u32 lun, u32 op, u32 lba, u32 count);
void usb_mass_disk_read(u32 lun, u32 lba, u32 count);
void usb_mass_disk_write(u32 lun, u32 lba, u32 count);

void usb_mass_send_csw(u32 status, u32 next)
{
    csw[12] = (unsigned char)status;
    CSW_SIG = 0x53425355u;
    usb_mass_write(1, csw, 13);
    mass_stage = next ? 4 : 5;
}
