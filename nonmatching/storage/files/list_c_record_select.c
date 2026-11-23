/*
 * list_c_record_select -- stock 0x40036EA4 (208 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Looks for the record of key v in the data block SYS+0xD744 (records of word SYS+0x3DA544 halfwords, SYS+0x758 of them; key = low byte of halfword 0 << 8 | low byte of halfword 1). When found: SYS+0xF9C = its key, word SYS+0x3DA53C = halfwords 2 and 3 packed the same way, and that many halfwords from record + 8 bytes are copied to SYS+0x3DA548.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"

#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_c_record_select(u32 v)
{
    volatile unsigned short *p = (volatile unsigned short *)(0x40544E68u + 0xD744u);
    int st = SI(f_3da544), n = st * SI(f_758), i;
    for (i = 0; i < n; i += st, p += st) {
        if (((u32)((p[0] << 8) & 0xFF00) | (p[1] & 0xFF)) == v) {
            u32 k = (u32)(short)p[0] << 8, c;
            SI(f_f9c) = k;
            SI(f_f9c) = (p[1] & 0xFF) | k;
            c = (p[2] << 8) & 0xFF00;
            SI(f_3da53c) = c;
            c |= p[3] & 0xFF;
            SI(f_3da53c) = c;
            copy_u16((unsigned short *)(0x40544E68u + 0x3DA548u), (const unsigned short *)(p + 4), c);
            return;
        }
    }
}
