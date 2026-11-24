/*
 * list_b_prune_missing -- stock 0x400364E0 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the last entry down: builds its path (0x40035BC4) and, when f_open of it fails, removes the entry (0x40035A58); then saves and reloads the index and clamps the cursor.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_b_prune_missing(void)
{
    unsigned char fil[0x250];
    int i;
    for (i = SI(list_count) - 1; i >= 0; i--) {
        u32 v = SYS16(0xFA6 + 2 * i);
        waveform_path_make(v);
        if (f_open(fil, PATHBUF, 1) == 0)
            f_close(fil);
        else
            list_b_entry_remove(v);
    }
    list_b_index_save();
    list_b_index_load();
    list_cursor_clamp_400362f4();
}
