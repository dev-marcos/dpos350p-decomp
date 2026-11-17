/*
 * list_b_marks_clear -- stock 0x400363CC (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the page marks SYS+0x84D6 (count / page size + 1 of them) and the entry marks SYS+0x5DC6 (count of them).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
int f_read(void *fp, void *buff, u32 btr, u32 *br);
int f_write(void *fp, const void *buff, u32 btw, u32 *bw);
int f_lseek(void *fp, unsigned long long ofs);
int f_unlink(const char *path);

void list_b_marks_clear(void)
{
    int i;
    for (i = 0; i < SS16(0xFA0) / (int)SYS16(0xF9A) + 1; i++)
        SYS8(0x84D6 + i) = 0;
    for (i = 0; i < SS16(0xFA0); i++)
        SYS8(0x5DC6 + i) = 0;
}
