/*
 * reload_chain_40035744 -- stock 0x40035744 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x400358B8, 0x400363CC, SYS+0xF90 = 0, u16 SYS+0xFA4 = 0, u16 SYS+0xF9E = 0, then 0x400362F4, 0x4003656C, 0x400365F4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "system.h"
#include "sys_info.h"

void reload_chain_40035744(void)
{
    list_b_index_load();
    list_b_marks_clear();
    SI(list_mark_mode) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
    list_cursor_clamp_400362f4();
    list_b_page_validate();
    list_b_page_thumbs();
}
