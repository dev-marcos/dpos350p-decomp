/*
 * list_c_index_load -- stock 0x40036B00 (220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads the index file 0x4008E2EC: the count (offset 0), the u16 SYS+0x3DA52E (offset 2) and SYS+0x758 * 2 bytes of entries (offset 4). When it does not open, list_c_index_create. Then the page count SYS+0xFA2 = count / page size, rounded up.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_c_index_load(void)
{
    unsigned char fil[0x250];
    int c, ps, q;
    if (f_open(fil, (const char *)IMG(4008e2ec), 1) != 0) {
        list_c_index_create();
    } else {
        f_lseek(fil, 0);
        f_read(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
        f_lseek(fil, 2);
        f_read(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
        f_lseek(fil, 4);
        f_read(fil, (void *)(0x40544E68u + 0xFA6u), SI(capture_slot_count) * 2u, 0);
        f_close(fil);
    }
    c = SI(list_count);
    ps = SI(list_page_size);
    q = c / ps;
    if (c - ps * q)
        q++;
    SI(list_page_count) = q;
}
