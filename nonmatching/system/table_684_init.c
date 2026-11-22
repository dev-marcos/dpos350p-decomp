/*
 * table_684_init -- stock 0x40063A50 (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the 64 bytes SYS+0x684..0x6C3 with fixed values (0x40..0x55; stored byte by byte by the stock, here as 16 words). Their meaning is OPEN.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void table_684_init(void)
{
    volatile u32 *p = (volatile u32 *)(0x40544E68u + 0x684);
    p[0] = 0x5252534Du;
    p[1] = 0x4045484Cu;
    p[2] = 0x53545550u;
    p[3] = 0x42484A51u;
    p[4] = 0x52515350u;
    p[5] = 0x4045494Du;
    p[6] = 0x40444846u;
    p[7] = 0x343A3C40u;
    p[8] = 0x40414039u;
    p[9] = 0x4046464Bu;
    p[10] = 0x403A3B43u;
    p[11] = 0x40434442u;
    p[12] = 0x403A463Au;
    p[13] = 0x404C4C4Eu;
    p[14] = 0x40434649u;
    p[15] = 0x40464652u;
}
