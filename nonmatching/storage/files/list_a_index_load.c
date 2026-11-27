/*
 * list_a_index_load -- stock 0x4003486C (180 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads the index file 0x4008DB60: the count into SYS+0xFA0 (2 bytes at offset 0) and SYS+0x754 * 2 bytes of entries from offset 2 into SYS+0xFA6. When it does not open, 0x400347A0. Then the page count SYS+0xFA2 = count / page size, rounded up.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_a_index_load(void)
{
    unsigned char fil[0x250];
    int c, ps, q;
    if (f_open(fil, (const char *)IMG(4008db60), 1) != 0) {
        list_a_index_create();
    } else {
        f_lseek(fil, 0);
        f_read(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
        f_lseek(fil, 2);
        f_read(fil, (void *)(0x40544E68u + 0xFA6u), SI(f_754) * 2u, 0);
        f_close(fil);
    }
    c = SI(list_count);
    ps = SI(list_page_size);
    q = c / ps;
    if (c - ps * q)
        q++;
    SI(list_page_count) = q;
}
