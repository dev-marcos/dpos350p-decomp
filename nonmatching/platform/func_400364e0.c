/*
 * list_b_prune_missing -- stock 0x400364E0 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the last entry down: builds its path (0x40035BC4) and, when f_open of it fails, removes the entry (0x40035A58); then saves and reloads the index and clamps the cursor.
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

void func_40035bc4(u32 v);
void list_b_entry_remove(u32 v);
void list_b_index_save(void);
void list_b_index_load(void);
void list_cursor_clamp_400362f4(void);

void list_b_prune_missing(void)
{
    unsigned char fil[0x250];
    int i;
    for (i = SS16(0xFA0) - 1; i >= 0; i--) {
        u32 v = SYS16(0xFA6 + 2 * i);
        func_40035bc4(v);
        if (f_open(fil, PATHBUF, 1) == 0)
            f_close(fil);
        else
            list_b_entry_remove(v);
    }
    list_b_index_save();
    list_b_index_load();
    list_cursor_clamp_400362f4();
}
