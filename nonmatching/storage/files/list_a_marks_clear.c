/*
 * list_a_marks_clear -- stock 0x40035320 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the page marks SYS+0x84D6 (count / page size + 1 of them) and the entry marks SYS+0x5DC6 (count of them).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "sys_info.h"

#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void list_a_marks_clear(void)
{
    int i;
    for (i = 0; i < SI(list_count) / (int)SI(list_page_size) + 1; i++)
        SYS8(0x84D6 + i) = 0;
    for (i = 0; i < SI(list_count); i++)
        SYS8(0x5DC6 + i) = 0;
}
