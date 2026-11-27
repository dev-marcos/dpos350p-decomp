/*
 * screenshot_path_make -- stock 0x40034B78 (796 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes the file name '/Screenshots/<n>.bmp' to SYS+0x88C0: the 13 bytes at 0x4008F82C ('/Screenshots' and its 0), the 0 replaced by '/', then n in decimal (no leading zeros) and '.bmp'. n < 10 is one character even when negative (the stock code adds '0'); for n >= 65536 only the '/' is written and the name is not terminated.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "storage/files.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

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

void screenshot_path_make(int n)
{
    __aeabi_memcpy4((void *)(SYS + 0x88C0), (const void *)IMG(4008f82c), 13);
    SI(f_88cc) = '/';
    path_number(SYS + 0x88CC, n, ".bmp");
}
