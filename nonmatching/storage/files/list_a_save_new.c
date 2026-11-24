/*
 * list_a_save_new -- stock 0x4003506C (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves a screenshot: screenshot_thumb_make, shows a message box (text pointer word at 0x42C3C994) at the screen centre plus its offsets, reloads the index, takes the next free number, builds its path and writes the file (bmp_save); when that returns non-zero the number is inserted and the index saved. Ends with rect_42e68c38_draw.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "ui/draw.h"
#include "ui/screens.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_a_save_new(void)
{
    const signed char *t = (const signed char *)(0x42C2FE74u + 0xCB14u);
    int x = SI(plot_x0) + (SI(plot_width) >> 1);
    int y = SI(plot_y0) + (SI(plot_height) >> 1);
    screenshot_thumb_make();
    message_box_draw(x + t[0], y + t[1], *(const u32 *)(t + 0xC), 0);
    list_a_index_load();
    list_a_next_free();
    screenshot_path_make(SI(f_f9c));
    if (bmp_save(PATHBUF)) {
        list_fa4_insert_754(SI(f_f9c));
        list_a_index_save();
    }
    rect_42e68c38_draw();
}
