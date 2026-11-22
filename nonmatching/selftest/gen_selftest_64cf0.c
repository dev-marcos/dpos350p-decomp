/*
 * gen_selftest_64cf0 -- stock 0x40064CF0 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Second generator self-test: settings 0x1797D51 / 1300 (with byte +0x2EFA = 0 and word 0x42E42888 = 0 first), 0x1279611 / 1500, 0xBC84B1 / 1200, 0x5DA66D / 1200, each applied with gen_update and checked by 0x400647F4 (0x15, 4, 1), (0x12, 5, 1), (0x16, 6, 2), (0x17, 7, 3); returns bit 0 of the AND of the four results.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void gen_update(void);
int lane_trim_search4x2(u32 a, u32 b, u32 c);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))

int gen_selftest_64cf0(void)
{
    u32 a, b, c;
    GB8(0x2EFA) = 0;
    GB32(0x2F04) = 0;
    GB32(0x2EFC) = 0x1797D51;
    GB32(0x2F00) = 1300;
    gen_update();
    a = lane_trim_search4x2(0x15, 4, 1);
    GB32(0x2EFC) = 0x1279611;
    GB32(0x2F00) = 1500;
    gen_update();
    a &= lane_trim_search4x2(0x12, 5, 1);
    GB32(0x2EFC) = 0xBC84B1;
    GB32(0x2F00) = 1200;
    gen_update();
    b = lane_trim_search4x2(0x16, 6, 2);
    GB32(0x2EFC) = 0x5DA66D;
    GB32(0x2F00) = 1200;
    gen_update();
    c = lane_trim_search4x2(0x17, 7, 3);
    return c & b & a & 1;
}
