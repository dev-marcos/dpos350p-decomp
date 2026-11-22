/*
 * scsi_invalid_cmd -- stock 0x4006DC90 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unsupported command: ILLEGAL REQUEST / INVALID COMMAND OPERATION CODE.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/mass_storage.h"

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

void scsi_invalid_cmd(u32 lun)
{
    (void)lun;
    debug_printf_stub(STR(0x4008EFC8));
    usb_mass_set_sense(CBW_LUN, 5, 0x20);
    usb_mass_send_csw(1, 0);
}
