/*
 * list_a_index_create -- stock 0x400347A0 (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Run when the index file does not open: creates the directory 0x4008F82C, opens 0x4008DB60 (mode 7), sets its attributes (f_chmod 2/2 and 4/4), zeroes SYS+0x754 + 2 entries and the count, and writes the empty index.
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

void list_a_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)0x4008F82Cu);
    f_open(fil, (const char *)0x4008DB60u, 7);
    f_chmod((const char *)0x4008DB60u, 2, 2);
    f_chmod((const char *)0x4008DB60u, 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SYS16(0x754) + 2);
    SS16(0xFA0) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x754) * 2u, 0);
    f_close(fil);
}
