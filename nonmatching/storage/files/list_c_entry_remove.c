/*
 * list_c_entry_remove -- stock 0x40036F74 (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Finds the entry equal to v among the first SYS+0x758 entries (index 0 when absent), shifts the following ones down, zeroes the last one and decrements the count SYS+0xFA0 when it is not zero.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_c_entry_remove(u32 v)
{
    int n = SI(capture_slot_count), i, k = 0, c;
    for (i = 0; i < n; i++) {
        if (SYS16(0xFA6 + 2 * i) == v) {
            k = i;
            break;
        }
    }
    for (; k < SI(capture_slot_count) - 1; k++)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA8 + 2 * k);
    SYS16(0xFA4 + 2 * SI(capture_slot_count)) = 0;
    c = SI(list_count);
    if (c)
        SI(list_count) = c - 1;
}
