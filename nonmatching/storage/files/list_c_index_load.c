/*
 * list_c_index_load -- stock 0x40036B00 (220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads the index file 0x4008E2EC: the count (offset 0), the u16 SYS+0x3DA52E (offset 2) and SYS+0x758 * 2 bytes of entries (offset 4). When it does not open, list_c_index_create. Then the page count SYS+0xFA2 = count / page size, rounded up.
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

void list_c_index_create(void);

void list_c_index_load(void)
{
    unsigned char fil[0x250];
    int c, ps, q;
    if (f_open(fil, (const char *)0x4008E2ECu, 1) != 0) {
        list_c_index_create();
    } else {
        f_lseek(fil, 0);
        f_read(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
        f_lseek(fil, 2);
        f_read(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
        f_lseek(fil, 4);
        f_read(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x758) * 2u, 0);
        f_close(fil);
    }
    c = SS16(0xFA0);
    ps = SYS16(0xF9A);
    q = c / ps;
    if (c - ps * q)
        q++;
    SS16(0xFA2) = q;
}
