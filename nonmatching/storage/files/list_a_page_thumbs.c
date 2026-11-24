/*
 * list_a_page_thumbs -- stock 0x40035578 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For up to 3 x 5 entries of the current page (stopping at the first zero one): builds the path (0x40034B78), loads the file (list_a_thumb_load) and copies u16 SYS+0x109684 * u16 SYS+0x109686 halfwords from SYS+0x109684 to slot n (SYS+0x12ED5C + n * 0x256D8).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_a_page_thumbs(void)
{
    int k = SI(list_page_size) * SI(list_page_index), n = 0, row, col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 5; col++) {
            u32 v = SYS16(0xFA6 + 2 * k);
            if (v == 0)
                return;
            screenshot_path_make(v);
            list_a_thumb_load();
            k++;
            copy_u16((unsigned short *)(0x40544E68u + 0x12ED5Cu + (u32)(unsigned short)n * 0x256D8u),
                     (const unsigned short *)(0x40544E68u + 0x109684u), SI(bmp_width) * SI(bmp_height));
            n++;
        }
    }
}
