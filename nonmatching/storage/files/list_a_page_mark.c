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
#include "sys_info.h"
#include "shared/storage.h"

void list_a_page_mark(void)
{
    int ps, start, end, lim;
    SYS8(0x84D6 + SI(list_page_index)) = 1;
    ps = SI(list_page_size);
    start = ps * SI(list_page_index);
    end = ps + start;
    lim = SI(list_count);
    if (lim > end)
        lim = end;
    if (start < lim)
        __aeabi_memset_entry((void *)(0x40544E68u + 0x5DC6u + start), lim - start, 1);
}
