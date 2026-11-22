/*
 * list_fa4_insert_756 -- stock 0x40035A00 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Shifts the u16 entries SYS+0xFA6.. up by one (u16 SYS+0x756 entries, from the top down), puts v at SYS+0xFA6 and increments the s16 count SYS+0xFA0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))

void list_fa4_insert_756(u32 v)
{
    int k;
    for (k = (int)SYS16(0x756) - 0; k >= 1; k--)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA4 + 2 * k);
    SYS16(0xFA6) = v;
    SS16(0xFA0) = SS16(0xFA0) + 1;
}
