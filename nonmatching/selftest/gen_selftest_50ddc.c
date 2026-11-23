/*
 * gen_selftest_50ddc -- stock 0x40050DDC (504 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test (DERIVED): both channels on with SYS+0x46 = SYS+0x10A = 2, byte 0x42E68BC6 = 0; one generator setting (block 0x42E3F984: byte +0x2EFA = 0, words +0x2EFC = 0x2160EC0, +0x2F00 = 1000, +0x2F04 = 0; SYS+0x588 = 0xE, SYS+0x41 = SYS+0x105 = 6), applied, two acquisitions. Pass when 5 * (s64 SYS+0x910 / 1000) and 5 * (SYS+0x970 / 1000) are below 4 * 1000 (64-bit compare): bytes 0x42E68B23 / 0x42E68B24, their AND in 0x42E68B22.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "generator.h"
#include "selftest.h"
#include "system.h"
#include "sys_info.h"

#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))

void gen_selftest_50ddc(void)
{
    int amp = 1000, q1, q2;
    u32 a, b;
    acq_defaults_reset();
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(f_046) = 2;
    SI(f_10a) = 2;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    GB8(0x2EFA) = 0;
    GB32(0x2EFC) = 0x2160EC0;
    GB32(0x2F00) = (u32)amp;
    GB32(0x2F04) = 0;
    SI(timebase_index) = 0xE;
    SI(ch1_vdiv_index) = 6;
    SI(ch2_vdiv_index) = 6;
    outputs_apply();
    gen_update();
    acquisition_dispatcher();
    acquisition_dispatcher();
    q1 = (int)(SI(f_970) / 1000);
    q2 = (int)(SI(f_910) / 1000);
    a = (long long)q2 * 5 < (long long)amp * 4;
    b = (long long)q1 * 5 < (long long)amp * 4;
    RS8(0x43) = a;
    RS8(0x44) = b;
    RS8(0x42) = b & a;
}
