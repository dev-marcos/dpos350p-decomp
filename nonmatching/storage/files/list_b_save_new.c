/*
 * list_b_save_new -- stock 0x400360F0 (220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves a waveform: when disk_free_bytes is below twice word SYS+0x774, shows the message box of the text word 0x42C3D820 at the screen centre (offsets at 0x42C3D828) and waits 1 s; otherwise reloads the index, takes the next free number, waveform_thumb_make, shows the message box of the text word 0x42C3C7F4, builds the path, encodes the settings image and writes it (file_write_360404); on success the number is inserted and the index saved. Ends with rect_42e68c38_draw.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "fatfs_app.h"
#include "storage/diskio.h"
#include "storage/files.h"
#include "storage/settings.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_b_save_new(void)
{
    const unsigned char *p = (const unsigned char *)(0x42C2FE74u + 0xC980u);
    int x = SI(plot_x0) + (SI(plot_width) >> 1);
    int y = SI(plot_y0) + (SI(plot_height) >> 1);
    if (disk_free_bytes() < SI(rec_hij_end) * 2u) {
        const signed char *t = (const signed char *)(0x42C2FE74u + 0xD9B4u);
        message_box_draw(x + t[0], y + t[1], *(const u32 *)(p + 0xEA0), 0);
        delay_ms(1000);
    } else {
        list_b_index_load();
        list_b_next_free();
        waveform_thumb_make();
        message_box_draw(x + (signed char)p[0x194], y + (signed char)p[0x195], *(const u32 *)p, 0);
        waveform_path_make(SI(f_f9c));
        settings_image_encode();
        if (file_write_360404(PATHBUF)) {
            list_fa4_insert_756(SI(f_f9c));
            list_b_index_save();
        }
    }
    rect_42e68c38_draw();
}
