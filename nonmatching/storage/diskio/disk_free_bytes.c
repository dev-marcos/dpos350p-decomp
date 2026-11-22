/*
 * disk_free_bytes -- stock 0x40049BE4 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Free space of "0:" in bytes (free clusters x sectors per cluster x 512), 0 on error.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/diskio.h"

u32 disk_free_bytes(void)
{
    u32 n;
    void *fs = g_fatfs0;
    if (f_getfree(STR_DRIVE0, &n, &fs) != 0)
        return 0;
    return ((*(unsigned short *)((u32)fs + 0x0A) * n) >> 1) << 10;
}
