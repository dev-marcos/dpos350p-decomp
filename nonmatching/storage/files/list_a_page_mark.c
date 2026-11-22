/*
 * list_a_page_mark -- stock 0x400352A8 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Marks the current page: byte SYS+0x84D6+page = 1, and the per-entry marks SYS+0x5DC6 of the entries of that page (up to the count) are set to 1 with __aeabi_memset.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "stock_asm.h"
#include "storage/files.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_a_page_mark(void)
{
    int ps, start, end, lim;
    SYS8(0x84D6 + SS16(0xF9E)) = 1;
    ps = SYS16(0xF9A);
    start = ps * SS16(0xF9E);
    end = ps + start;
    lim = SS16(0xFA0);
    if (lim > end)
        lim = end;
    if (start < lim)
        __aeabi_memset_entry((void *)(0x40544E68u + 0x5DC6u + start), lim - start, 1);
}
