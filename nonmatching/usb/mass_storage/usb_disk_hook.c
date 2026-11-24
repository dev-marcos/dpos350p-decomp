/*
 * usb_disk_hook -- stock 0x4006A734 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Empty hook (bx lr), tail-called by usb_disk_sd_read / usb_disk_sd_write when a transfer ends.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "usb/mass_storage.h"
#include "shared/access.h"

#define ST 0x42E68D1Bu               /* data stage state: 0 idle, 1 busy */
#define ERR 0x42E68D1Cu              /* error bits: 8 read, 4 write */
#define CSWST 0x42E68D1Du            /* command state: 3 send status, 4 host ended the data */
#define USB 0x42E68C10u              /* +8: data residue */
#define BUF 0x42E08074u              /* sector bounce buffer */

void usb_disk_hook(void)
{
}
