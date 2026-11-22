/*
 * disk_check_file -- stock 0x40051C28 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Storage check of the POST: disk_format_and_reload, writes the 30-byte string at 0x4008DA20 into the file 0x4008E364 (`sd_nand_check_file.bin`), reads it back, deletes the file and sets byte 0x42E68AF0 to 1 when the two strings are equal (strcmp), 0 otherwise.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void __aeabi_memcpy(void *d, const void *s, u32 n);
void disk_format_and_reload(void);
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
int f_read(void *fp, void *buff, u32 btr, u32 *br);
int f_write(void *fp, const void *buff, u32 btw, u32 *bw);
int f_lseek(void *fp, unsigned long long ofs);
int f_unlink(const char *path);
int strcmp(const char *a, const char *b);

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
