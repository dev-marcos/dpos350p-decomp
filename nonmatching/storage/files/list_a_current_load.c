/*
 * list_a_current_load -- stock 0x40035624 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the path of the selected entry (SYS+0xFA4) and reads 0x12B6A0 bytes from offset 0xDEA8 of that file to 0x42468A58.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_a_current_load(void)
{
    unsigned char fil[0x250];
    screenshot_path_make(SYS16(0xFA6 + 2 * SS16(0xFA4)));
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0xDEA8);
    f_read(fil, (void *)0x42468A58u, 0x12B6A0, 0);
    f_close(fil);
}
