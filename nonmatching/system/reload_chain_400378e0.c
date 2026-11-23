/*
 * reload_chain_400378e0 -- stock 0x400378E0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40036B00, 0x40036BDC, 0x400377F8, 0x4003771C, then SYS+0xF90 = 0, u16 SYS+0xFA4 = 0, u16 SYS+0xF9E = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "system.h"
#include "sys_info.h"

void reload_chain_400378e0(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_marks_clear();
    list_cursor_clamp_4003771c();
    SI(list_mark_mode) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
}
