/*
 * list_b_file_load -- stock 0x40036260 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Opens the file at SYS+0x88C0, reads its 0x1C-byte header into SYS+0x360404 and decodes it (settings_h_decode), then reads word SYS+0x76C bytes from offset SYS+0x760 to the same place in the block and decodes them (settings_i_decode).
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

void settings_h_decode(void);
void settings_i_decode(void);

void list_b_file_load(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), 0x1C, 0);
    settings_h_decode();
    f_lseek(fil, SYS32(0x760));
    f_read(fil, (void *)(0x40544E68u + 0x360404u + SYS32(0x760)), SYS32(0x76C), 0);
    f_close(fil);
    settings_i_decode();
}
