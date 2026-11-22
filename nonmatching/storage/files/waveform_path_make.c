/*
 * waveform_path_make -- stock 0x40035BC4 (796 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Same as screenshot_path_make for '/Waveform/<n>.wav': the 10 bytes at 0x4008FFB4 ('/Waveform' and its 0) to SYS+0x88C0, '/' at SYS+0x88C9, then the number and '.wav'.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "storage/files.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))

/* p points at the '/' before the number; n < 10 is written as one character, also when it is
   negative; 10..65535 as 2..5 decimal digits; from 65536 on only the '/' is there */
static void path_number(u32 p, int n, const char *ext)
{
    int nd, i;
    if (n < 10) {
        B8(p + 1) = (unsigned char)(n + '0');
        nd = 1;
    } else {
        if (n < 100)
            nd = 2;
        else if (n < 1000)
            nd = 3;
        else if (n < 10000)
            nd = 4;
        else if (n < 65536)
            nd = 5;
        else
            return;
        for (i = nd; i >= 1; i--) {
            B8(p + i) = (unsigned char)('0' + n % 10);
            n /= 10;
        }
    }
    for (i = 0; i < 4; i++)
        B8(p + 1 + nd + i) = (unsigned char)ext[i];
    B8(p + 5 + nd) = 0;
}

void waveform_path_make(int n)
{
    __aeabi_memcpy4((void *)(SYS + 0x88C0), (const void *)0x4008FFB4u, 10);
    B8(SYS + 0x88C9) = '/';
    path_number(SYS + 0x88C9, n, ".wav");
}
