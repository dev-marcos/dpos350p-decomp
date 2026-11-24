/*
 * list_c_index_save -- stock 0x40036C54 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes the index file 0x4008E2EC (opened read/write): the count at offset 0, the u16 SYS+0x3DA52E at offset 2 and SYS+0x758 * 2 bytes of entries at offset 4. When it does not open, 0x40036958.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_c_index_save(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x4008E2ECu, 3) != 0) {
        list_c_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
    f_lseek(fil, 4);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SI(f_758) * 2u, 0);
    f_close(fil);
}
