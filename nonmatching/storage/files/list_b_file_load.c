/*
 * list_b_file_load -- stock 0x40036260 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Opens the file at SYS+0x88C0, reads its 0x1C-byte header into SYS+0x360404 and decodes it (settings_h_decode), then reads word SYS+0x76C bytes from offset SYS+0x760 to the same place in the block and decodes them (settings_i_decode).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "storage/settings.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_b_file_load(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), 0x1C, 0);
    settings_h_decode();
    f_lseek(fil, SI(rec_i_start));
    f_read(fil, (void *)(0x40544E68u + 0x360404u + SI(rec_i_start)), SI(rec_i_size), 0);
    f_close(fil);
    settings_i_decode();
}
