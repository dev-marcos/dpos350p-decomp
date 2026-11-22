/*
 * adc_lanes_config_for_rate -- stock 0x40063F04 (1592 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sets the ADC lane configuration for the sample rate U64 SYS+0x538 (mSa/s): the lane masks SYS+0x674..+0x67B, the per-lane delays SYS+0x67C..+0x683 (copied from the 8-byte group of table A SYS+0x684 for that rate, or 0x40 for an unknown rate), the per-lane phases SYS+0x6D4..+0x6DB (0) and the lane -> bank map SYS+0x6DC..+0x6E3. With divisor SYS+0x1B0 = 1 the rates are 1000 / 950 / 475 / 237.5 MSa/s (groups 0..3), otherwise 475 / 380 / 237.5 / 118.75 MSa/s (groups 4..7).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))

#define R_1000 0xE8D4A51000ULL
#define R_950 0xDD30699C00ULL
#define R_475 0x6E9834CE00ULL
#define R_237 0x374C1A6700ULL
#define R_380 0x5879C3D800ULL
#define R_118 0x1BA60D3380ULL

static void masks(u32 m0, u32 m1, u32 m2, u32 m3, u32 m4, u32 m5, u32 m6, u32 m7)
{
    B8(SYS + 0x674) = (unsigned char)m0;
    B8(SYS + 0x675) = (unsigned char)m1;
    B8(SYS + 0x676) = (unsigned char)m2;
    B8(SYS + 0x677) = (unsigned char)m3;
    B8(SYS + 0x678) = (unsigned char)m4;
    B8(SYS + 0x679) = (unsigned char)m5;
    B8(SYS + 0x67A) = (unsigned char)m6;
    B8(SYS + 0x67B) = (unsigned char)m7;
}

static void delays(u32 grp)
{
    u32 i;
    for (i = 0; i < 8; i++)
        B8(SYS + 0x67C + i) = B8(SYS + 0x684 + 8 * grp + i);
}

static void banks(u32 b0, u32 b1, u32 b2, u32 b3, u32 b4, u32 b5, u32 b6, u32 b7)
{
    W32(SYS + 0x6D4) = 0;
    W32(SYS + 0x6D8) = 0;
    B8(SYS + 0x6DC) = (unsigned char)b0;
    B8(SYS + 0x6DD) = (unsigned char)b1;
    B8(SYS + 0x6DE) = (unsigned char)b2;
    B8(SYS + 0x6DF) = (unsigned char)b3;
    B8(SYS + 0x6E0) = (unsigned char)b4;
    B8(SYS + 0x6E1) = (unsigned char)b5;
    B8(SYS + 0x6E2) = (unsigned char)b6;
    B8(SYS + 0x6E3) = (unsigned char)b7;
}

void adc_lanes_config_for_rate(void)
{
    unsigned long long rate = U64(SYS + 0x538);
    u32 i;
    if (B8(SYS + 0x1B0) == 1) {
        if (rate == R_1000 || rate == R_950) {
            masks(0xF0, 0x3C, 0x0F, 0xC3, 0x78, 0x1E, 0x87, 0xE1);
            delays(rate == R_1000 ? 0 : 1);
            banks(2, 6, 3, 7, 0, 4, 1, 5);
        } else if (rate == R_475) {
            masks(0xF0, 0x1E, 0x0F, 0xC3, 0x78, 0x3C, 0x87, 0xE1);
            delays(2);
            banks(2, 6, 3, 7, 0, 4, 5, 1);
        } else {
            masks(0xF0, 0x1E, 0x0F, 0xE1, 0x78, 0x3C, 0x87, 0xC3);
            if (rate == R_237)
                delays(3);
            banks(2, 6, 7, 3, 0, 4, 5, 1);
        }
        return;
    }
    if (rate == R_475) {
        masks(0xF0, 0x3C, 0x0F, 0xC3, 0x78, 0x1E, 0x87, 0xE1);
        delays(4);
    } else {
        masks(0xF0, 0x3C, 0x0F, 0xC3, 0xF0, 0x3C, 0x0F, 0xC3);
        if (rate == R_380)
            delays(5);
        else if (rate == R_237)
            delays(6);
        else if (rate == R_118)
            delays(7);
        else
            for (i = 0; i < 8; i++)
                B8(SYS + 0x67C + i) = 0x40;
    }
    banks(2, 3, 0, 1, 6, 7, 4, 5);
}
