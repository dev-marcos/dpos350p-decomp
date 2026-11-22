/*
 * list_b_index_create -- stock 0x400357EC (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Run when the index file does not open: creates the directory 0x4008FFB4, opens 0x4008E2D0 (mode 7), sets its attributes (f_chmod 2/2 and 4/4), zeroes SYS+0x756 + 2 entries and the count, and writes the empty index.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_b_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)0x4008FFB4u);
    f_open(fil, (const char *)0x4008E2D0u, 7);
    f_chmod((const char *)0x4008E2D0u, 2, 2);
    f_chmod((const char *)0x4008E2D0u, 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SYS16(0x756) + 2);
    SS16(0xFA0) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x756) * 2u, 0);
    f_close(fil);
}
