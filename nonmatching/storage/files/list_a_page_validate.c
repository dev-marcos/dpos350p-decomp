/*
 * list_a_page_validate -- stock 0x400354C0 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Page index = selected entry SYS+0xFA4 / page size; walks the entries of that page until a zero entry, and when one of them has no file (screenshot_file_exists), runs list_a_prune_missing and stops. Then the cursor clamp.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_a_page_validate(void)
{
    volatile unsigned short *p;
    int i;
    SI(list_page_index) = SI(list_selected) / (int)SI(list_page_size);
    p = (volatile unsigned short *)(0x40544E68u + 0xFA6u + 2u * (u32)(SI(list_page_size) * SI(list_page_index)));
    for (i = 0; i < SI(list_page_size); i++) {
        u32 v = *p++;
        if (v == 0)
            break;
        if (screenshot_file_exists(v) == 0) {
            list_a_prune_missing();
            break;
        }
    }
    list_a_cursor_clamp();
}
