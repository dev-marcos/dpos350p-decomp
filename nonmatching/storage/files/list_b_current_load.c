/*
 * list_b_current_load -- stock 0x400366A0 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the path of the selected entry (SYS+0xFA4); when the file opens: state_restore_all when SYS+0x665 is set, state_backup_all, the 0x1C-byte header into SYS+0x360404 (settings_h_decode), then word SYS+0x774 bytes from offset 0 into the same block (settings_image_decode); finally SYS+0x665 = SYS+0x3C = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "storage/settings.h"
#include "system.h"
#include "util.h"
#include "sys_info.h"

#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_b_current_load(void)
{
    unsigned char fil[0x250];
    waveform_path_make(SYS16(0xFA6 + 2 * SI(list_selected)));
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    if (SI(f_665))
        state_restore_all();
    state_backup_all();
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), 0x1C, 0);
    settings_h_decode();
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), SI(rec_hij_end), 0);
    f_close(fil);
    settings_image_decode();
    SI(f_665) = 1;
    SI(acq_transition_flag) = 1;
}
