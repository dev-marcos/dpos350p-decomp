/*
 * list_c_insert -- stock 0x40036D88 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Shifts the u16 entries SYS+0xFA6.. up by one (u16 SYS+0x758 minus 1 entries, from the top down), puts v at SYS+0xFA6 and increments the s16 count SYS+0xFA0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "sys_info.h"


void list_c_insert(u32 v)
{
    int k;
    for (k = (int)SI(capture_slot_count) - 1; k >= 1; k--)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA4 + 2 * k);
    SI(f_fa6) = v;
    SI(list_count) = SI(list_count) + 1;
}
