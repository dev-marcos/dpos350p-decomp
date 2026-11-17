/*
 * list_c_index_create -- stock 0x40036958 (424 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Run when a list C file does not open: creates 0x4008FA2C, writes an empty index 0x4008E2EC (count 0, SYS+0x3DA52E = 1, zeroed entries) and a data file 0x4008E8D4 of SYS+0x758 * SYS+0x3DA544 halfwords. The data block is cleared with copy_u16 from address 0, as the firmware does.
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

void list_c_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)0x4008FA2Cu);
    f_chmod((const char *)0x4008FA2Cu, 2, 2);
    f_chmod((const char *)0x4008FA2Cu, 4, 4);
    f_open(fil, (const char *)0x4008E2ECu, 0xB);
    f_chmod((const char *)0x4008E2ECu, 2, 2);
    f_chmod((const char *)0x4008E2ECu, 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SYS16(0x758));
    SYS16(0x3DA52E) = 1;
    SS16(0xFA0) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
    f_lseek(fil, 4);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x758) * 2u, 0);
    f_close(fil);
    f_open(fil, (const char *)0x4008E8D4u, 0xB);
    f_chmod((const char *)0x4008E8D4u, 2, 2);
    f_chmod((const char *)0x4008E8D4u, 4, 4);
    copy_u16((unsigned short *)(0x40544E68u + 0xD744u), 0, SYS16(0x758) * SYS32(0x3DA544));
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xD744u), SYS16(0x758) * SYS32(0x3DA544), 0);
    f_close(fil);
}
