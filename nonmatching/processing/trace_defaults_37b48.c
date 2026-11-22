/*
 * trace_defaults_37b48 -- stock 0x40037B48 (224 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: sets a group of SYS flags and values to their defaults (0x3E, 0x3F, 0x102, 0x103, 0x46, 0x10A, 0x54F, 0x274, 0x275 = 1; words 0x60, 0x120 = 250; 0x2A9 = 0x21; 0x660 = 8; 0x70D pulsed 1 then 0), touch byte +0x16 = 1, 0x42E4287E = 0, 0x42E4288C = 500, and builds the table at 0x42E554B0: start 1,000,000, end 50,000,000, step 200,000, the values from +0x10 and the count at +0xC.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

#define H32(o) (*(volatile u32 *)(0x42E554B0u + (o)))

void trace_defaults_37b48(void)
{
    u32 i, v;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    SYS8(0x3E) = 1;
    SYS8(0x3F) = 1;
    SYS32(0x60) = 0xFA;
    SYS8(0x102) = 1;
    SYS8(0x103) = 1;
    SYS32(0x120) = 0xFA;
    SYS8(0x46) = 1;
    SYS8(0x10A) = 1;
    SYS8(0x54F) = 1;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 1;
    SYS8(0x2A9) = 0x21;
    SYS8(0x70D) = 1;
    SYS8(0x70D) = 0;
    SYS8(0x660) = 8;
    SYS8(0x275) = 1;
    SYS8(0x274) = 1;
    *(volatile unsigned char *)(0x42E3F984u + 0x2EFA) = 0;
    *(volatile u32 *)(0x42E3F984u + 0x2F08) = 500;
    H32(4) = 50000000;
    H32(0) = 1000000;
    H32(8) = 200000;
    i = 0;
    v = 1000000;
    do {
        H32(0xC) = i;
        H32(0x10 + 4 * i) = v;
        v += H32(8);
        i = H32(0xC) + 1;
    } while (H32(4) >= v);
    H32(0xC) = i;
}
