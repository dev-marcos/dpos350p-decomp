/*
 * fft_input_prepare_b -- stock 0x40061580 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * n = word +0xF8 of block 0x42E68668 consecutive samples from the middle of the capture buffer 0x42AC7CF4, shifted right by SYS+0x264, as floats into 0x42D164B4, followed by n zeros (when n < 2n as u32).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

static u32 shr(u32 v, u32 s)
{
    s &= 0xFF;
    return s < 32 ? v >> s : 0;
}

void fft_input_prepare_b(void)
{
    u32 n = *(volatile u32 *)(0x42E68668u + 0xF8), i;
    volatile float *d = (volatile float *)0x42D164B4u;
    if (n) {
        volatile unsigned short *s = (volatile unsigned short *)0x42AC7CF4u + ((SI(f_454) - n) >> 1);
        u32 sh = SI(f_264);
        for (i = 0; i < n; i++)
            d[i] = (float)(int)shr(s[i], sh);
    }
    if (n < (u32)(n << 1)) {
        for (i = 0; i < n; i++)
            d[n + i] = 0.0f;
    }
}
