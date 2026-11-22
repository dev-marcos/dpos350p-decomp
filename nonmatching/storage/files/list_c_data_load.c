/*
 * list_c_data_load -- stock 0x40036BDC (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The file 0x4008E8D4 (mode 1) at offset 0: f_read of word SYS+0x3DA544 * u16 SYS+0x758 bytes at SYS+0xD744. When it does not open, 0x40036958.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_c_data_load(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x4008E8D4u, 1) != 0) {
        list_c_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0xD744u), SYS32(0x3DA544) * SYS16(0x758), 0);
    f_close(fil);
}
