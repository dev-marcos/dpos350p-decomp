/*
 * gen_selftest_50bac -- stock 0x40050BAC (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test (DERIVED): range SYS+0x588 = 0x10, levels SYS+0x41 = SYS+0x105 = 0xB, SYS+0x54F = 1. For four output settings (channel 1 on with SYS+0x46 = 0 then 1; channel 2 on with SYS+0x10A = 0 then 1) it applies the outputs, runs one acquisition, then 30 acquisitions summing the spread (max - min, as a byte) of the capture bytes 0x42DF76D4[0x14 .. SYS+0x640 - 0x28). Pass when the sum with the setting = 1 is below 0.9f times the sum with 0, per channel: bytes 0x42E68B20 / 0x42E68B21, their AND in 0x42E68B1F.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "selftest.h"
#include "system.h"
#include "sys_info.h"

#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define CAP8(o) (*(volatile unsigned char *)(0x42DF76D4u + (o)))

static u32 spread_sum(int end)
{
    u32 s = 0, k;
    acquisition_dispatcher();
    for (k = 0; k < 30; k++) {
        u32 mx = 0, mn = 0xFF;
        int j;
        acquisition_dispatcher();
        for (j = 0x14; j < end; j++) {
            u32 b = CAP8(j);
            if (mx < b)
                mx = b;
            if (b < mn)
                mn = b;
        }
        s += (mx - mn) & 0xFF;
    }
    return s;
}

void gen_selftest_50bac(void)
{
    int end, a, b, c, d;
    u32 p1, p2;
    acq_defaults_reset();
    SI(f_54f) = 1;
    SI(timebase_index) = 0x10;
    SI(ch1_vdiv_index) = 0xB;
    SI(ch2_vdiv_index) = 0xB;
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 0;
    SI(f_046) = 0;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    outputs_apply();
    end = (int)SI(f_640) - 0x28;
    a = (int)spread_sum(end);
    SI(f_046) = 1;
    outputs_apply();
    b = (int)spread_sum(end);
    SI(ch1_enabled) = 0;
    SI(ch2_enabled) = 1;
    SI(f_10a) = 0;
    outputs_apply();
    c = (int)spread_sum(end);
    SI(f_10a) = 1;
    outputs_apply();
    d = (int)spread_sum(end);
    p1 = b < (int)((float)a * 0.9f);
    p2 = d < (int)((float)c * 0.9f);
    RS8(0x40) = p1;
    RS8(0x41) = p2;
    RS8(0x3F) = p2 & p1;
}
