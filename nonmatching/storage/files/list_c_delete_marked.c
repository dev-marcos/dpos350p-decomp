/*
 * list_c_delete_marked -- stock 0x40037460 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the last entry down: for each marked entry (SYS+0x5DC6+i), list_current_advance(i), list_c_entry_remove and record_clear of its value. Then list_a_marks_clear, the index and data saved and reloaded, SYS+0xF90 = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void record_clear_raw(u32 v) __asm__("record_clear");

void list_c_delete_marked(void)
{
    int i;
    for (i = SI(list_count) - 1; i >= 0; i--) {
        if (SYS8(0x5DC6 + i)) {
            u32 v = SYS16(0xFA6 + 2 * i);
            list_current_advance((unsigned short)i);
            list_c_entry_remove(v);
            record_clear_raw(v);
        }
    }
    list_a_marks_clear();
    list_c_index_save();
    list_c_data_save();
    list_c_index_load();
    list_c_data_load();
    SI(list_mark_mode) = 0;
}
