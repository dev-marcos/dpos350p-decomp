/*
 * gen_selftest_64c28 -- stock 0x40064C28 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator self-test: four generator settings (words 0x42E42880 / 0x42E42884 of block 0x42E3F984 = 0x2D32355 / 1800, 0x2D32355 / 1800, 0x1797D51 / 1300, 0xBC84B1 / 1300; first also byte +0x2EFA = 0 and word 0x42E42888 = 0), each applied with gen_update and checked by 0x4006453C (0xB, 0, 1), (0x15, 1, 1), (0x16, 2, 1), (0x17, 3, 2); returns 1 when all four pass (bit 0 of the AND).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void gen_update(void);
int lane_trim_search8(u32 a, u32 b, u32 c);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))

int gen_selftest_64c28(void)
{
    u32 a, b, c;
    GB8(0x2EFA) = 0;
    GB32(0x2EFC) = 0x2D32355;
    GB32(0x2F04) = 0;
    GB32(0x2F00) = 1800;
    gen_update();
    a = lane_trim_search8(0xB, 0, 1);
    GB32(0x2EFC) = 0x2D32355;
    GB32(0x2F00) = 1800;
    gen_update();
    a &= lane_trim_search8(0x15, 1, 1);
    GB32(0x2EFC) = 0x1797D51;
    GB32(0x2F00) = 1300;
    gen_update();
    b = lane_trim_search8(0x16, 2, 1);
    GB32(0x2EFC) = 0xBC84B1;
    GB32(0x2F00) = 1300;
    gen_update();
    c = lane_trim_search8(0x17, 3, 2);
    return c & b & a & 1;
}
