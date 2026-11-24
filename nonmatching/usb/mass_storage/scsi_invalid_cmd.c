/*
 * scsi_invalid_cmd -- stock 0x4006DC90 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unsupported command: ILLEGAL REQUEST / INVALID COMMAND OPERATION CODE.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_invalid_cmd(u32 lun)
{
    (void)lun;
    debug_printf_stub(STR(0x4008EFC8));
    usb_mass_set_sense(CBW_LUN, 5, 0x20);
    usb_mass_send_csw(1, 0);
}
