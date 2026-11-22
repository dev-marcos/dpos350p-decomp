/*
 * list_b_index_save -- stock 0x4003596C (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes the index file 0x4008E2D0 (opened read/write): the count at offset 0 and SYS+0x756 * 2 bytes of entries at offset 2. When it does not open, 0x400357EC.
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

void list_b_index_save(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x4008E2D0u, 3) != 0) {
        list_b_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x756) * 2u, 0);
    f_close(fil);
}
