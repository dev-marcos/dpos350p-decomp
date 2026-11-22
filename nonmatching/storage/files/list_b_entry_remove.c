/*
 * list_b_entry_remove -- stock 0x40035A58 (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Finds the entry equal to v among the first SYS+0x756 entries (index 0 when absent), shifts the following ones down, zeroes the last one and decrements the count SYS+0xFA0 when it is not zero.
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
int f_mkdir(const char *path);
int f_chmod(const char *path, unsigned char attr, unsigned char mask);
void fill_u16(unsigned short *d, unsigned short v, u32 n);

void list_b_entry_remove(u32 v)
{
    int n = SYS16(0x756), i, k = 0, c;
    for (i = 0; i < n; i++) {
        if (SYS16(0xFA6 + 2 * i) == v) {
            k = i;
            break;
        }
    }
    for (; k < SYS16(0x756) - 1; k++)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA8 + 2 * k);
    SYS16(0xFA4 + 2 * SYS16(0x756)) = 0;
    c = SS16(0xFA0);
    if (c)
        SS16(0xFA0) = c - 1;
}
