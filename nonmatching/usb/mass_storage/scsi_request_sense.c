/*
 * scsi_request_sense -- stock 0x4006DA14 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * REQUEST SENSE: at most 18 bytes of sense_data.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_request_sense(u32 lun)
{
    u32 n = cbw[0x13];
    (void)lun;
    if ((int)n >= 0x13) n = 0x12;
    usb_mass_send_data(sense_data, n);
}
