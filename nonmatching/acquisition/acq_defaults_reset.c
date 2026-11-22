/*
 * acq_defaults_reset -- stock 0x4003EB40 (488 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Resets the acquisition state in SYS_INFO to its defaults and ends with runtime_state_commit_force (tail call). Notable values: time-base index SYS+0x588 = 0x17 and +0x598 = 0x16, the 64-bit words +0x570 / +0x578 = 0, range bytes +0x41 / +0x105 = 6, coupling bytes +0x46 / +0x10A = 0, the trigger level copies +0x27E, +0x60 and +0x120 = u16 +0xE, the cursor words +0x808 = u16 +8 / 5, +0x80C = 4 x (u16 +8) / 5, +0x810 = u16 +0xA / 4, +0x814 = 3 x (u16 +0xA) / 4, the saved-trace state SYS+0x3DA52C (byte 0, u16 +4 = 0, +8 = u16 +8 / 3, +0xC = 2 x (u16 +8) / 3), key repeat counters +0x86A..+0x870 = 0, 0, 1, 1; touch byte +0x16 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

#define SYS 0x40544E68u
#define GEN 0x42E3F984u
#define TOUCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define D64(a) (*(volatile unsigned long long *)(a))

void acq_defaults_reset(void)
{
    u32 lvl = H16(SYS + 0xE), w8, wa, t;
    W32(SYS + 0x5F0) = 0;
    B8(SYS + 0x588) = 0x17;
    B8(SYS + 0x598) = 0x16;
    W32(SYS + 0x570) = 0;
    W32(SYS + 0x574) = 0;
    D64(SYS + 0x578) = 0;
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x2AA) = 0;
    B8(SYS + 0x3E) = 1;
    H16(SYS + 0x27E) = (unsigned short)lvl;
    B8(SYS + 0x42) = 0;
    B8(SYS + 0x46) = 0;
    B8(SYS + 0x3F) = 0;
    B8(SYS + 0x41) = 6;
    W32(SYS + 0x60) = lvl;
    B8(SYS + 0x102) = 0;
    B8(SYS + 0x106) = 0;
    B8(SYS + 0x10A) = 0;
    B8(SYS + 0x103) = 0;
    B8(SYS + 0x105) = 6;
    W32(SYS + 0x120) = lvl;
    B8(SYS + 0x274) = 1;
    B8(SYS + 0x275) = 1;
    B8(SYS + 0x27A) = 0;
    B8(SYS + 0x27C) = 0;
    B8(SYS + 0x27B) = 0;
    B8(SYS + 0x284) = 0;
    W32(SYS + 0x64C) = 0;
    B8(SYS + 0x70E) = 0;
    B8(SYS + 0x54F) = 0;
    B8(SYS + 0x54E) = 0;
    B8(SYS + 0x654) = 0x32;
    B8(SYS + 0x672) = 0x14;
    B8(SYS + 0x656) = 0x1E;
    w8 = H16(SYS + 8);
    B8(SYS + 0x673) = 2;
    B8(SYS + 0x3DA52C) = 0;
    H16(SYS + 0x3DA530) = 0;
    W32(SYS + 0x3DA534) = (w8 * 0xAAABu) >> 17;
    W32(SYS + 0x3DA538) = (u32)(((long long)(int)(w8 * 2) * 0x55555556LL) >> 32);
    B8(SYS + 0x2A) = 0;
    B8(SYS + 0x28) = 0;
    B8(SYS + 0x101) = 1;
    H16(SYS + 0x3A) = 0;
    W32(SYS + 0x808) = (w8 * 0xCCCDu) >> 18;
    wa = H16(SYS + 0xA);
    W32(SYS + 0x710) = 0;
    B8(SYS + 0x653) = 0;
    B8(SYS + 0x655) = 1;
    B8(SYS + 0x660) = 0;
    W32(SYS + 0x810) = wa >> 2;
    B8(SYS + 0x650) = 1;
    B8(SYS + 0x651) = 1;
    t = 3 * wa;
    H16(SYS + 0x86A) = 0;
    H16(SYS + 0x86C) = 0;
    H16(SYS + 0x86E) = 1;
    H16(SYS + 0x870) = 1;
    W32(SYS + 0x874) = 0;
    B8(SYS + 0x580) = 1;
    B8(SYS + 0x70D) = 1;
    H16(SYS + 0x586) = 0;
    H16(SYS + 0x664) = 0;
    H16(SYS + 0x666) = 0;
    B8(SYS + 0x671) = 0;
    B8(SYS + 0x652) = 0;
    B8(SYS + 0x77A) = 1;
    W32(SYS + 0x658) = 0;
    W32(SYS + 0x780) = 0;
    W32(SYS + 0x784) = 0;
    W32(SYS + 0x788) = 0;
    B8(SYS + 0x802) = 0;
    B8(SYS + 0x803) = 0;
    W32(SYS + 0x80C) = (u32)(((long long)(int)(w8 * 4) * 0x66666667LL) >> 32) >> 1;
    W32(SYS + 0x814) = (t + ((t >> 1) >> 30)) >> 2;
    B8(SYS + 0x804) = 0;
    W32(SYS + 0x878) = 0;
    B8(TOUCH + 0x16) = 1;
    runtime_state_commit_force();
}
