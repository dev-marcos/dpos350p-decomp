/*
 * list_a_reload -- stock 0x40035540 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reloads list A: index, marks cleared, SYS+0xF90 = 0 with the u16 SYS+0xFA4 and SYS+0xF9E, cursor clamp, page check; then falls through into list_a_page_thumbs.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "sys_info.h"

void list_a_reload(void)
{
    list_a_index_load();
    list_a_marks_clear();
    SI(list_mark_mode) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
    list_cursor_clamp_40035248();
    list_a_page_validate();
    list_a_page_thumbs();
}
