/*
 * list_b_current_load -- stock 0x400366A0 (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the path of the selected entry (SYS+0xFA4); when the file opens: state_restore_all when SYS+0x665 is set, state_backup_all, the 0x1C-byte header into SYS+0x360404 (settings_h_decode), then word SYS+0x774 bytes from offset 0 into the same block (settings_image_decode); finally SYS+0x665 = SYS+0x3C = 1.
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

void waveform_path_make(u32 v);
void settings_h_decode(void);
void settings_image_decode(void);

void list_b_current_load(void)
{
    unsigned char fil[0x250];
    waveform_path_make(SYS16(0xFA6 + 2 * SS16(0xFA4)));
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    if (SYS8(0x665))
        state_restore_all();
    state_backup_all();
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), 0x1C, 0);
    settings_h_decode();
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), SYS32(0x774), 0);
    f_close(fil);
    settings_image_decode();
    SYS8(0x665) = 1;
    SYS8(0x3C) = 1;
}
