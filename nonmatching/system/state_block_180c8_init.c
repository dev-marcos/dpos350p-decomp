/*
 * state_block_180c8_init -- stock 0x4003ED80 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Defaults of the block at 0x42E180C8 (+0x4C/+0x54 = 1000, +0x50 = 0, +0x58 = 50000000, +0x5C = 20, bytes +0xAC60 and +0xAD18 = 0, +0xAC64 = h / 5 and +0xAC68 = 4h / 5 with h = the halfword +4) and the byte 0x42E68BC6 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "state/bode_block.h"

#define B(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))
#define W(o) (*(volatile u32 *)(0x42E180C8u + (o)))

void state_block_180c8_init(void)
{
    u32 h;
    BODES(freq_count) = 20;
    h = *(volatile unsigned short *)(0x42E180C8u + 4);
    W(0x58) = 50000000;
    W(0x54) = 1000;
    W(0x4C) = 1000;
    BODES(cursors_on) = 0;
    W(0x50) = 0;
    B(0xAC60 + 0xB8) = 0;
    W(0xAC64) = h / 5u;
    W(0xAC68) = (u32)((int)(h * 4) / 5);
    *(volatile unsigned char *)0x42E68BC6u = 1;
}
