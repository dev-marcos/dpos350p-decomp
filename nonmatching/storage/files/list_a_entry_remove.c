/*
 * list_a_entry_remove -- stock 0x40034A0C (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Finds the entry equal to v among the first SYS+0x754 entries (index 0 when absent), shifts the following ones down, zeroes the last one and decrements the count SYS+0xFA0 when it is not zero.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"

#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_a_entry_remove(u32 v)
{
    int n = SI(f_754), i, k = 0, c;
    for (i = 0; i < n; i++) {
        if (SYS16(0xFA6 + 2 * i) == v) {
            k = i;
            break;
        }
    }
    for (; k < SI(f_754) - 1; k++)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA8 + 2 * k);
    SYS16(0xFA4 + 2 * SI(f_754)) = 0;
    c = SI(list_count);
    if (c)
        SI(list_count) = c - 1;
}
