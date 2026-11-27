/*
 * usb_mass_out_handler -- stock 0x4006BA98 (1460 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * OUT packet: in stage 0 parse a CBW (the CBW decoder at 0x4006BB6C is a separate stock function that the map merges into this one), in stage 1 continue a WRITE(10); otherwise phase error.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

/* stock 0x4006BB6C: decode a CBW (only reached by a tail branch from usb_mass_out_handler) */
static void usb_mass_decode_cbw(void)
{
    u32 len = mass_rx_len, i, asc;
    if (len) {
        unsigned char *s = mass_rx_ptr, *d = cbw;
        for (i = len; i; i--)
            *d++ = *s++;
    }
    CSW_TAG = *(u32 *)(cbw + 4);
    CSW_RESIDUE = CBW_DLEN;
    if (len != 31) {
        debug_printf_stub(STR(4008f8dc));
        CBW_SIG = 0;
        asc = 0x1A;
        goto fail;
    }
    if (CBW_OP == 0x28 || CBW_OP == 0x2A) {
        mass_lba = (u32)cbw[0x11] << 24 | (u32)cbw[0x12] << 16 | (u32)cbw[0x13] << 8 | cbw[0x14];
        mass_count = (u32)cbw[0x16] << 8 | cbw[0x17];
    }
    if (CBW_SIG != 0x43425355u) {
        debug_printf_stub(STR(4008f91c));
        asc = 0x20;
        goto fail;
    }
    if (max_lun < CBW_LUN || CBW_CBLEN == 0 || (int)CBW_CBLEN >= 0x11) {
        debug_printf_stub(STR(4008f8ec));
        asc = 0x24;
        goto fail;
    }
    debug_printf_stub(STR(4008f8fc), CBW_OP);
    switch (CBW_OP) {
    case 0x00: scsi_test_unit_ready(CBW_LUN); return;
    case 0x03: scsi_request_sense(CBW_LUN); return;
    case 0x04: scsi_format_unit(CBW_LUN); return;
    case 0x12: scsi_inquiry(CBW_LUN); return;
    case 0x1A: scsi_mode_sense6(CBW_LUN); return;
    case 0x1B:
    case 0x1E: scsi_start_stop(CBW_LUN); return;
    case 0x23: scsi_read_format_capacities(CBW_LUN); return;
    case 0x25: scsi_read_capacity10(CBW_LUN); return;
    case 0x28: scsi_read10(CBW_LUN, mass_lba, mass_count); return;
    case 0x2A: scsi_write10(CBW_LUN, mass_lba, mass_count); return;
    case 0x2F: scsi_verify10(CBW_LUN); return;
    case 0x5A: scsi_mode_sense10(CBW_LUN); return;
    case 0x08: case 0x0A: case 0x15: case 0x1D: case 0x55: case 0x88: case 0x8A: case 0x8F:
    case 0x9E: case 0xA8: case 0xAA: case 0xAF:
        scsi_invalid_cmd(CBW_LUN); return;
    }
    debug_printf_stub(STR(4008f90c));
    asc = 0x20;
fail:
    usb_mass_set_sense(CBW_LUN, 5, asc);
    usb_mass_send_csw(1, 0);
}

void usb_mass_out_handler(unsigned char *buf, u32 len)
{
    u32 fl = mass_flags | 0x10, lun;
    mass_flags = (unsigned char)fl;
    mass_rx_len = (unsigned short)len;
    mass_rx_ptr = buf;
    if (mass_stage == 0) {
        usb_mass_decode_cbw();
        return;
    }
    if (mass_stage == 1) {
        lun = CBW_LUN;
        if (CBW_OP == 0x2A) {
            mass_flags = (unsigned char)(fl | 1);
            scsi_write10(lun, mass_lba, mass_count);
            return;
        }
        usb_mass_set_sense(lun, 5, 0x24);
    } else {
        usb_mass_set_sense(CBW_LUN, 5, 0x24);
    }
    usb_mass_send_csw(2, 0);
}
