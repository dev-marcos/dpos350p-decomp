/*
 * list_b_index_load -- stock 0x400358B8 (180 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads the index file 0x4008E2D0: the count into SYS+0xFA0 (2 bytes at offset 0) and SYS+0x756 * 2 bytes of entries from offset 2 into SYS+0xFA6. When it does not open, 0x400357EC. Then the page count SYS+0xFA2 = count / page size, rounded up.
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

void list_b_index_create(void);

void list_b_index_load(void)
{
    unsigned char fil[0x250];
    int c, ps, q;
    if (f_open(fil, (const char *)0x4008E2D0u, 1) != 0) {
        list_b_index_create();
    } else {
        f_lseek(fil, 0);
        f_read(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
        f_lseek(fil, 2);
        f_read(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x756) * 2u, 0);
        f_close(fil);
    }
    c = SS16(0xFA0);
    ps = SYS16(0xF9A);
    q = c / ps;
    if (c - ps * q)
        q++;
    SS16(0xFA2) = q;
}
