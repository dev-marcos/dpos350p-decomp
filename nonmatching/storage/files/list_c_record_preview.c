/*
 * list_c_record_preview -- stock 0x4003754C (460 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Preview of the selected list C record: list_c_record_select(a), then with len = SI(f_3da53c) cells at SYS+0x3DA548 and f = len / 193 + 1: m = SI(f_554) * f * 193 / len, array_3da548_decimate(f), len /= f; the cells are copied to the middle of the buffer 0x416431E8 (around element 10000), stretched there to 193 samples (simple_integer_index_mapping_nearest_neighbour), and len = len * m / SI(f_554) cells are copied back. The minimum (over cells 1 .. len-2) is subtracted from every cell, and the cells are scaled to 8 + 122 * v / (max - min) into the preview block (u16 count at SYS+0x109684, values from SYS+0x109688). A length of 0 is not tested (the stock code divides by it and then loops 2^32 - 2 times).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define LEN W32(SYS + 0x3DA53Cu)
#define CELLS (SYS + 0x3DA548u)
#define ZBUF 0x416431E8u
#define ZMID (ZBUF + 0x4E20u)

void list_c_record_preview(u32 a)
{
    u32 len, f, m, n, i, mx, mn;
    double sc;
    list_c_record_select(a);
    len = LEN;
    f = (u32)(((unsigned long long)len * 0xA9C84A48u) >> 32 >> 7) + 1;
    m = (u32)__aeabi_uldivmod((unsigned long long)SI(f_554) * f * 193u,
                              (unsigned long long)(long long)(int)len);
    array_3da548_decimate((unsigned char)f);
    n = LEN / f;
    LEN = n;
    copy_u16((unsigned short *)(ZMID - 2 * ((int)n / 2)), (const unsigned short *)CELLS, n);
    simple_integer_index_mapping_nearest_neighbour(ZBUF, 10000, 193, m, 0, 0);
    n = (u32)__aeabi_uldivmod((unsigned long long)(long long)(int)LEN * m, (unsigned long long)SI(f_554));
    LEN = n;
    copy_u16((unsigned short *)CELLS, (const unsigned short *)(ZMID - 2 * ((int)n / 2)), n);
    n = LEN;
    mx = 0;
    mn = 0xFFFFFFFFu;
    if (n - 1 >= 2)
        for (i = 0; i < n - 2; i++) {
            u32 v = H16(CELLS + 2 + 2 * i);
            if (v >= mx)
                mx = v;
            if (v <= mn)
                mn = v;
        }
    if (n != 0) {
        i = 0;
        do {
            H16(CELLS + 2 * i) = (unsigned short)(H16(CELLS + 2 * i) - mn);
            i++;
        } while (i < LEN);
    }
    sc = 122.0 / (double)(mx - mn);
    for (i = 0; i < LEN; i++)
        H16(SYS + 0x109688 + 2 * i) = (unsigned short)(int)(8.0 + (double)H16(CELLS + 2 * i) * sc);
    SI(bmp_width) = (unsigned short)LEN;
}
