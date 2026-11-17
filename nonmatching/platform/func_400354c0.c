/*
 * list_a_page_validate -- stock 0x400354C0 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Page index = selected entry SYS+0xFA4 / page size; walks the entries of that page until a zero entry, and when one of them has no file (file_exists_40034b78), runs list_a_prune_missing and stops. Then the cursor clamp.
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

u32 file_exists_40034b78(u32 v);
void list_a_prune_missing(void);
void list_cursor_clamp_40035248(void);

void list_a_page_validate(void)
{
    volatile unsigned short *p;
    int i;
    SS16(0xF9E) = SS16(0xFA4) / (int)SYS16(0xF9A);
    p = (volatile unsigned short *)(0x40544E68u + 0xFA6u + 2u * (u32)(SYS16(0xF9A) * SS16(0xF9E)));
    for (i = 0; i < SYS16(0xF9A); i++) {
        u32 v = *p++;
        if (v == 0)
            break;
        if (file_exists_40034b78(v) == 0) {
            list_a_prune_missing();
            break;
        }
    }
    list_cursor_clamp_40035248();
}
