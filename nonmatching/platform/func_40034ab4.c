/*
 * list_a_next_free -- stock 0x40034AB4 (196 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the used table SYS+0x8924 (SYS+0x754 halfwords), sets used[e] = 1 for each non-zero entry e, then stores in SYS+0xF9C the first number from 1 whose slot is free (unchanged when none is).
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

void list_a_next_free(void)
{
    int i, n;
    for (i = 0; i < SYS16(0x754); i++)
        SYS16(0x8924 + 2 * i) = 0;
    for (i = 0; i < SS16(0xFA0); i++) {
        u32 e = SYS16(0xFA6 + 2 * i);
        if (e)
            SYS16(0x8924 + 2 * e) = 1;
    }
    n = SYS16(0x754);
    for (i = 1; i < n; i++) {
        if (SYS16(0x8924 + 2 * i) == 0) {
            SYS16(0xF9C) = i;
            return;
        }
    }
}
