/*
 * disk_check_file -- stock 0x40051C28 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Storage check of the POST: disk_format_and_reload, writes the 30-byte string at 0x4008DA20 into the file 0x4008E364 (`sd_nand_check_file.bin`), reads it back, deletes the file and sets byte 0x42E68AF0 to 1 when the two strings are equal (strcmp), 0 otherwise.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "libc.h"
#include "storage/diskio.h"
#include "ui/input.h"
#include "util.h"

void disk_check_file(void)
{
    char rd[0x20], wr[0x20];
    unsigned char fil[0x250];
    const char *path = (const char *)0x4008E364u;
    __aeabi_memcpy(wr, (const void *)0x4008DA20u, 0x1E);
    disk_format_and_reload();
    f_open(fil, path, 0xB);
    f_lseek(fil, 0);
    f_write(fil, wr, 0x1E, 0);
    f_close(fil);
    f_open(fil, path, 3);
    f_lseek(fil, 0);
    f_read(fil, rd, 0x1E, 0);
    f_close(fil);
    f_unlink(path);
    *(volatile unsigned char *)0x42E68AF0u = strcmp(wr, rd) == 0;
}
