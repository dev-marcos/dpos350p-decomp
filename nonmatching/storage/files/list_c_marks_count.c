/*
 * list_c_marks_count -- stock 0x40037880 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * u16 SYS+0x88BE = the number of non-zero bytes among the first s16 SYS+0xFA0 bytes at SYS+0x5DC6 (the store is repeated each step, as in the stock code).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "sys_info.h"


void list_c_marks_count(void)
{
    volatile unsigned char *p = (volatile unsigned char *)(0x40544E68u + 0x5DC6u);
    int n = SI(list_count);
    u32 c = 0;
    for (; n >= 1; n--) {
        SI(list_marked_count) = c;
        if (*p++)
            c++;
    }
    SI(list_marked_count) = c;
}
