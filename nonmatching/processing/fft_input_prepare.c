/*
 * fft_input_prepare -- stock 0x40041A4C (340 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FFT input for the enabled channels (SYS+0x3E/+0x42, SYS+0x102/+0x106): n = word SYS+0x7F8 points taken every st = SYS+0x454 / n samples (at least 1) from the middle of the capture buffer (0x42AC7CF4, channel 2 at +0x62250), shifted right by SYS+0x264, as floats into 0x42E49EB4 / 0x42E4DEB4, followed by n zeros.
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

void fft_input_prepare(void)
{
    u32 n = SI(fft_size), q = n ? SI(block_len) / n : 0, off, i;
    int st = (int)q <= 0 ? 1 : (int)q;
    off = (SI(block_len) - st * n) >> 1;
    if (SI(ch1_enabled) && SI(ch1_fft_enabled)) {
        volatile unsigned short *s = (volatile unsigned short *)0x42AC7CF4u + off;
        volatile float *d = (volatile float *)0x42E49EB4u;
        u32 sh = SI(raw_shift);
        for (i = 0; i < n; i++, s += st)
            d[i] = (float)(int)shr(*s, sh);
        for (i = n; i < 2 * n; i++)
            d[i] = 0.0f;
    }
    if (SI(ch2_enabled) && SI(ch2_fft_enabled)) {
        volatile unsigned short *s = (volatile unsigned short *)(0x42AC7CF4u + off * 2u + 0x62250u);
        volatile float *d = (volatile float *)0x42E4DEB4u;
        u32 sh = SI(raw_shift);
        for (i = 0; i < n; i++, s += st)
            d[i] = (float)(int)shr(*s, sh);
        for (i = n; i < 2 * n; i++)
            d[i] = 0.0f;
    }
}
