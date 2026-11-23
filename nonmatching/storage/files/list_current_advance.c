/*
 * list_current_advance -- stock 0x40037418 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When the u16 SYS+0x3DA52E equals the entry SYS+0xFA6 + 2i: it becomes the next entry (SYS+0xFA8 + 2i) when i < count - 1, else the entry SYS+0xFA4 + 2i.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "sys_info.h"


void list_current_advance(int i)
{
    if (SYS16(0xFA6 + 2 * i) != SI(list_current))
        return;
    if (i < SI(list_count) - 1)
        SI(list_current) = SYS16(0xFA8 + 2 * i);
    else
        SI(list_current) = SYS16(0xFA4 + 2 * i);
}
