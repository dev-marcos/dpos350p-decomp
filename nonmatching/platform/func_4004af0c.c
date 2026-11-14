/*
 * post_channel_mux_check -- stock 0x4004AF0C (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * POST channel-multiplex test: both channels on at the given V/div, positions 0xAA then 0x1E; each time the measured mean position (SYS_INFO+0xB34 / +0xB50) must be within +-3 (+-6 at index 12). Returns 1 both pass, -2 CH1 only, -1 CH2 only, 0 neither.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int post_channel_mux_check(u32 vdiv)
{
    u32 tol = vdiv == 12 ? 6 : 3;
    unsigned char ok_hi1 = 0, ok_hi2 = 0, ok_lo1 = 0, ok_lo2 = 0;
    u32 a, m;
    SYS8(0x588) = 0x18; SYS8(0x41) = vdiv; SYS8(0x105) = vdiv;
    SYS8(0x3C) = 0; SYS8(0x3E) = 1; SYS8(0x40) = 0; SYS8(0x3F) = 0; SYS8(0x46) = 2;
    SYS8(0x102) = 1; SYS8(0x104) = 0; SYS8(0x103) = 0; SYS8(0x10A) = 2; SYS8(0x275) = 0; SYS8(0x660) = 0;
    SYS32(0x60) = 0xAA; SYS32(0x120) = 0xAA;
    func_4003e708(); delay_ms(100); acquisition_dispatcher();
    a = SYS32(0x60); m = SYS32(0xB34);
    if (a - tol < m && m < tol + a) ok_hi1 = 1;
    a = SYS32(0x120); m = SYS32(0xB50);
    if (a - tol < m && m < tol + a) ok_hi2 = 1;
    SYS32(0x60) = 0x1E; SYS32(0x120) = 0x1E;
    func_4003e708(); delay_ms(100); acquisition_dispatcher();
    a = SYS32(0x60); m = SYS32(0xB34);
    if (a - tol < m && m < tol + a) ok_lo1 = 1;
    a = SYS32(0x120); m = SYS32(0xB50);
    if (a - tol < m && m < tol + a) ok_lo2 = 1;
    {
        int c1 = (ok_lo1 & ok_hi1) != 0, c2 = (ok_lo2 & ok_hi2) != 0;
        if (c1 & c2) return 1;
        if (c1) return -2;
        if (c2) return -1;
        return 0;
    }
}
