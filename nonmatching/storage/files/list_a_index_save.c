/*
 * list_a_index_save -- stock 0x40034920 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes the index file 0x4008DB60 (opened read/write): the count at offset 0 and SYS+0x754 * 2 bytes of entries at offset 2. When it does not open, 0x400347A0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_a_index_save(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)IMG(4008db60), 3) != 0) {
        list_a_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SI(f_754) * 2u, 0);
    f_close(fil);
}
