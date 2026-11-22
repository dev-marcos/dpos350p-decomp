/*
 * acquisition_geometry_setup -- stock 0x4003C008 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sets the capture geometry: SYS+0x38 = 1; step SYS16+0x6F8 = 2 at time-base index 0x21, else 1; SYS16+0x6F6 = SYS+0x558 x step; per-bank count SYS16+0x6F4 = SYS16+0x6F6 / SYS+0x1B1 + 2 (copied to +0x640 and +0x6F0); SYS+0x708 = SYS16+0x6F6 + 1; SYS+0x454 = SYS+0x2B0, SYS+0x38C = SYS+0x2B0 / 2; SYS+0x568 / +0x56C = -SYS16+0xC; clears SYS+0x6FC, +0x700, +0x2C4, +0x63C. The stock division by zero gives 0; the C does the same explicitly.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void acquisition_geometry_setup(void)
{
    u32 d, q, prod, L;
    int b;
    SYS8(0x38) = 1;
    d = SYS8(0x1B1);
    SYS16(0x6F8) = SYS8(0x588) == 0x21 ? 2 : 1;
    prod = (SYS32(0x558) * SYS16(0x6F8)) & 0xFFFF;
    SYS16(0x6F6) = prod;
    b = (int)d;
    q = b ? (u32)((int)prod / b) : 0;
    SYS16(0x6F4) = q + 2;
    SYS32(0x6FC) = 0;
    SYS32(0x700) = 0;
    SYS16(0x640) = SYS16(0x6F4);
    SYS16(0x6F0) = SYS16(0x6F4);
    L = SYS32(0x2B0);
    SYS32(0x2C4) = 0;
    SYS32(0x63C) = 0;
    SYS32(0x708) = SYS16(0x6F6) + 1;
    SYS32(0x454) = L;
    SYS32(0x38C) = L >> 1;
    SYS32(0x568) = -(u32)SYS16(0xC);
    SYS32(0x56C) = -(u32)SYS16(0xC);
}
