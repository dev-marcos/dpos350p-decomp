/*
 * list_a_page_thumbs -- stock 0x40035578 (172 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For up to 3 x 5 entries of the current page (stopping at the first zero one): builds the path (0x40034B78), loads the file (list_a_thumb_load) and copies u16 SYS+0x109684 * u16 SYS+0x109686 halfwords from SYS+0x109684 to slot n (SYS+0x12ED5C + n * 0x256D8).
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

void screenshot_path_make(u32 v);
void list_a_thumb_load(void);

void list_a_page_thumbs(void)
{
    int k = SYS16(0xF9A) * SS16(0xF9E), n = 0, row, col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 5; col++) {
            u32 v = SYS16(0xFA6 + 2 * k);
            if (v == 0)
                return;
            screenshot_path_make(v);
            list_a_thumb_load();
            k++;
            copy_u16((unsigned short *)(0x40544E68u + 0x12ED5Cu + (u32)(unsigned short)n * 0x256D8u),
                     (const unsigned short *)(0x40544E68u + 0x109684u), SYS16(0x109684) * SYS16(0x109686));
            n++;
        }
    }
}
