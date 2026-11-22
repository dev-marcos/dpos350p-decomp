/*
 * state_block_3f984_init -- stock 0x4003ED28 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Defaults of the block at 0x42E3F984 (+1 = 0, +4 = pointer to +8, +8 = 0, +0xC = 0, +0x2EFA = 1, +0x2EFC/+0x2F00 = 1000, +0x2F04 = 0, +0x2F08 = 500, +0x2F60 = 0) and the byte 0x42E68BC6 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"

#define B(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define W(o) (*(volatile u32 *)(0x42E3F984u + (o)))

void state_block_3f984_init(void)
{
    B(1) = 0;
    W(4) = 0x42E3F984u + 8;
    B(0x2EFA) = 1;
    W(0x2EFC) = 1000;
    W(0x2F00) = 1000;
    W(8) = 0;
    W(0x2F04) = 0;
    B(0xC) = 0;
    B(0x2F60) = 0;
    W(0x2F08) = 500;
    *(volatile unsigned char *)0x42E68BC6u = 1;
}
