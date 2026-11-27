/*
 * list_a_index_create -- stock 0x400347A0 (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Run when the index file does not open: creates the directory 0x4008F82C, opens 0x4008DB60 (mode 7), sets its attributes (f_chmod 2/2 and 4/4), zeroes SYS+0x754 + 2 entries and the count, and writes the empty index.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_a_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)IMG(4008f82c));
    f_open(fil, (const char *)IMG(4008db60), 7);
    f_chmod((const char *)IMG(4008db60), 2, 2);
    f_chmod((const char *)IMG(4008db60), 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SI(f_754) + 2);
    SI(list_count) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SI(f_754) * 2u, 0);
    f_close(fil);
}
