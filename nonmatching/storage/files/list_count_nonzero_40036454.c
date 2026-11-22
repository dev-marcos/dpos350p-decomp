/*
 * list_count_nonzero_40036454 -- stock 0x40036454 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * u16 SYS+0x88BE = the number of non-zero bytes among the first s16 SYS+0xFA0 bytes at SYS+0x5DC6 (the store is repeated each step, as in the stock code).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))

void list_count_nonzero_40036454(void)
{
    volatile unsigned char *p = (volatile unsigned char *)(0x40544E68u + 0x5DC6u);
    int n = SS16(0xFA0);
    u32 c = 0;
    for (; n >= 1; n--) {
        SYS16(0x88BE) = c;
        if (*p++)
            c++;
    }
    SYS16(0x88BE) = c;
}
