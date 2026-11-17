/*
 * gen_selftest_4f89c -- stock 0x4004F89C (640 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test (DERIVED): range SYS+0x588 = 0x18, levels SYS+0x41 = SYS+0x105 = 6, SYS+0x660 = 5, byte 0x42E68BC6 = 0. Four output settings, each around the centre e = SYS+0xE with the step h = SYS+0x20 (offsets SYS+0x60 / SYS+0x120 = e +/- 2h in turn, channels SYS+0x3E / SYS+0x102 on and off), each applied and followed by two acquisitions. Checks of the measured words SYS+0xB34 and SYS+0xB50 against e * 1.3 and e * 0.7 (doubles); byte 0x42E68AF3 = 1 when all pass.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003eb40(void);
void outputs_apply(void);
void acquisition_dispatcher(void);

static void apply2(void)
{
    outputs_apply();
    acquisition_dispatcher();
    acquisition_dispatcher();
}

void gen_selftest_4f89c(void)
{
    u32 e, h, v, p1, p2, p3, p4;
    func_4003eb40();
    e = SYS16(0xE);
    SYS8(0x588) = 0x18;
    SYS8(0x41) = 6;
    SYS8(0x105) = 6;
    h = SYS16(0x20);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS32(0x120) = e - 2 * h;
    SYS32(0x60) = e + 2 * h;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    apply2();

    e = SYS16(0xE);
    p1 = (double)e * 1.3 < (double)SYS32(0xB34) && (double)SYS32(0xB50) < (double)e * 0.7;
    h = SYS16(0x20);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS32(0x60) = e - 2 * h;
    SYS32(0x120) = e + 2 * h;
    apply2();

    e = SYS16(0xE);
    p2 = (double)e * 1.3 < (double)SYS32(0xB50) && (double)SYS32(0xB34) < (double)e * 0.7;
    h = SYS16(0x20);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS32(0x60) = e + 2 * h;
    SYS32(0x120) = e - 2 * h;
    apply2();

    e = SYS16(0xE);
    v = SYS32(0xB34);
    h = SYS16(0x20);
    SYS8(0x3E) = 0;
    SYS8(0x102) = 1;
    SYS32(0x60) = e + 2 * h;
    SYS32(0x120) = e - 2 * h;
    p3 = (double)e * 1.3 < (double)v;
    apply2();

    e = SYS16(0xE);
    p4 = (double)SYS32(0xB50) < (double)e * 0.7;
    *(volatile unsigned char *)(0x42E68AE0u + 0x13) = p4 & (p3 & (p2 & p1));
}
