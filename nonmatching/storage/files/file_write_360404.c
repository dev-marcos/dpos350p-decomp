/*
 * file_write_360404 -- stock 0x40035780 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Creates the file at path (mode 0xB) and writes word SYS+0x774 bytes of the block at SYS+0x360404 from offset 0. Returns 1 when f_write succeeds, 0 otherwise (also when the file does not open).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

u32 file_write_360404(const char *path)
{
    unsigned char fil[0x250];
    u32 r = 0;
    if (f_open(fil, path, 0xB) == 0) {
        f_lseek(fil, 0);
        if (f_write(fil, (void *)(0x40544E68u + 0x360404u), SYS32(0x774), 0) == 0)
            r = 1;
        f_close(fil);
    }
    return r;
}
