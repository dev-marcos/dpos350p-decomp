/*
 * list_a_thumb_load -- stock 0x40035188 (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Opens the file at SYS+0x88C0 and reads the u16 at offset 0x46 into SYS+0x109684, the u16 at 0x48 into SYS+0x109686 and their product * 2 bytes from 0x4A into SYS+0x109688.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_a_thumb_load(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0x46);
    f_read(fil, (void *)(0x40544E68u + 0x109684u), 2, 0);
    f_lseek(fil, 0x48);
    f_read(fil, (void *)(0x40544E68u + 0x109686u), 2, 0);
    f_lseek(fil, 0x4A);
    f_read(fil, (void *)(0x40544E68u + 0x109688u), SYS16(0x109686) * SYS16(0x109684) * 2u, 0);
    f_close(fil);
}
