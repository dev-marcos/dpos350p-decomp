/*
 * gen_selftest_4f89c -- stock 0x4004F89C (640 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test (DERIVED): range SYS+0x588 = 0x18, levels SYS+0x41 = SYS+0x105 = 6, SYS+0x660 = 5, byte 0x42E68BC6 = 0. Four output settings, each around the centre e = SYS+0xE with the step h = SYS+0x20 (offsets SYS+0x60 / SYS+0x120 = e +/- 2h in turn, channels SYS+0x3E / SYS+0x102 on and off), each applied and followed by two acquisitions. Checks of the measured words SYS+0xB34 and SYS+0xB50 against e * 1.3 and e * 0.7 (doubles); byte 0x42E68AF3 = 1 when all pass.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "selftest.h"
#include "system.h"
#include "sys_info.h"

static void apply2(void)
{
    outputs_apply();
    acquisition_dispatcher();
    acquisition_dispatcher();
}

void gen_selftest_4f89c(void)
{
    u32 e, h, v, p1, p2, p3, p4;
    acq_defaults_reset();
    e = SI(level_default);
    SI(timebase_index) = 0x18;
    SI(ch1_vdiv_index) = 6;
    SI(ch2_vdiv_index) = 6;
    h = SI(f_020);
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(ch2_offset) = e - 2 * h;
    SI(ch1_offset) = e + 2 * h;
    SI(f_660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    apply2();

    e = SI(level_default);
    p1 = (double)e * 1.3 < (double)SI(f_b34) && (double)SI(f_b50) < (double)e * 0.7;
    h = SI(f_020);
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(ch1_offset) = e - 2 * h;
    SI(ch2_offset) = e + 2 * h;
    apply2();

    e = SI(level_default);
    p2 = (double)e * 1.3 < (double)SI(f_b50) && (double)SI(f_b34) < (double)e * 0.7;
    h = SI(f_020);
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 0;
    SI(ch1_offset) = e + 2 * h;
    SI(ch2_offset) = e - 2 * h;
    apply2();

    e = SI(level_default);
    v = SI(f_b34);
    h = SI(f_020);
    SI(ch1_enabled) = 0;
    SI(ch2_enabled) = 1;
    SI(ch1_offset) = e + 2 * h;
    SI(ch2_offset) = e - 2 * h;
    p3 = (double)e * 1.3 < (double)v;
    apply2();

    e = SI(level_default);
    p4 = (double)SI(f_b50) < (double)e * 0.7;
    *(volatile unsigned char *)(0x42E68AE0u + 0x13) = p4 & (p3 & (p2 & p1));
}
