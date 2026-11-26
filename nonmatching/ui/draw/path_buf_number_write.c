/*
 * path_buf_number_write -- stock 0x40037160 (640 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes n in decimal (no leading zeros) and a terminating 0 to SYS+0x88C0. n < 10 is one character even when negative (the stock code adds '0'); from 65536 on nothing is written.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

void path_buf_number_write(int n)
{
    int nd, i;
    if (n < 10) {
        SI(path_buf) = (unsigned char)(n + '0');
        SI(path_name) = 0;
        return;
    }
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
    B8(SYS + 0x88C0 + nd) = 0;
    for (i = nd - 1; i >= 0; i--) {
        B8(SYS + 0x88C0 + i) = (unsigned char)('0' + n % 10);
        n /= 10;
    }
}
