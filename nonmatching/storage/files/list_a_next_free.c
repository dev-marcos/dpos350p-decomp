/*
 * list_a_next_free -- stock 0x40034AB4 (196 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the used table SYS+0x8924 (SYS+0x754 halfwords), sets used[e] = 1 for each non-zero entry e, then stores in SYS+0xF9C the first number from 1 whose slot is free (unchanged when none is).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"

#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_a_next_free(void)
{
    int i, n;
    for (i = 0; i < SI(f_754); i++)
        SYS16(0x8924 + 2 * i) = 0;
    for (i = 0; i < SI(list_count); i++) {
        u32 e = SYS16(0xFA6 + 2 * i);
        if (e)
            SYS16(0x8924 + 2 * e) = 1;
    }
    n = SI(f_754);
    for (i = 1; i < n; i++) {
        if (SYS16(0x8924 + 2 * i) == 0) {
            SI(f_f9c) = i;
            return;
        }
    }
}
