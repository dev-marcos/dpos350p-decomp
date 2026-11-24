/* USB device state shared by the mass-storage (CBW / CSW, LUN, sense data) and CDC
 * (setup request, line coding) handlers (build 2025-06-19, see docs/DRIVERS.md). */
#ifndef SHARED_USB_H
#define SHARED_USB_H

#define CBW_CBLEN cbw[0x0E]
#define CBW_DLEN (*(u32 *)0x42E68B98u)
#define CBW_FLAGS cbw[0x0C]
#define CBW_LUN cbw[0x0D]
#define CBW_OP cbw[0x0F]
#define CBW_SIG (*(u32 *)0x42E68B90u)
#define CSW_RESIDUE (*(u32 *)0x42E68C18u)
#define CSW_SIG (*(u32 *)0x42E68C10u)
#define CSW_TAG (*(u32 *)0x42E68C14u)
#define cbw ((unsigned char *)0x42E68B90u)                   /* 31-byte command block wrapper */
#define csw ((unsigned char *)0x42E68C10u)                   /* 13-byte command status wrapper */
#define line_coding ((unsigned char *)0x40095484u)           /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
#define lun_blksize ((u32 *)0x42E68D0Cu)
#define lun_blocks ((u32 *)0x42E68D10u)
#define mass_count (*(u32 *)0x42E68CE8u)
#define mass_flags (*(unsigned char *)0x42E68D1Cu)
#define mass_lba (*(u32 *)0x42E68CE4u)
#define mass_rx_len (*(unsigned short *)0x42E68D16u)
#define mass_rx_ptr (*(unsigned char **)0x42E68CE0u)
#define mass_stage (*(unsigned char *)0x42E68D1Du)           /* 0 CBW, 1 data out, 2 data in, 3 sent, 4/5 CSW */
#define max_lun (*(volatile u32 *)0x42E68D23u)               /* read as an unaligned word by the CBW check */
#define max_lun_rsp (*(unsigned char *)0x42E68D1Eu)
#define sense_data ((unsigned char *)0x40095400u)            /* 18-byte fixed-format sense */
#define setup_req ((unsigned char *)0x42E68C58u)             /* copy of the last setup packet */

#endif /* SHARED_USB_H */
