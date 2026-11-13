/*
 * auto_off_check -- stock 0x4004A1F4 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When auto power-off is on and its deadline passed: draws the shutdown image at the screen centre, waits 3 s and powers off.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void auto_off_check(void)
{
    u32 cx = SYS16(4) + (SYS16(8) >> 1);
    u32 cy = SYS16(6) + (SYS16(0x0A) >> 1);
    if (SYS32(0x658) != 0 && SYS32(0x65C) < g_tick_count) {
        unsigned char *p = (unsigned char *)(0x42C2FE74u + 0xDB60);
        func_40034134(cx + (signed char)p[0x194], cy + (signed char)p[0x195], *(u32 *)p, 0);
        delay_ms(3000);
        power_off();
    }
}
