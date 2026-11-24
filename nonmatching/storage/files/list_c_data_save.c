/*
 * list_c_data_save -- stock 0x40036D10 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The file 0x4008E8D4 (mode 3) at offset 0: f_write of word SYS+0x3DA544 * u16 SYS+0x758 bytes at SYS+0xD744. When it does not open, 0x40036958.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_c_data_save(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x4008E8D4u, 3) != 0) {
        list_c_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xD744u), SI(capture_cells) * SI(capture_slot_count), 0);
    f_close(fil);
}
