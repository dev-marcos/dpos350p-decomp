/*
 * list_a_delete_marked -- stock 0x400350F4 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the last entry down: each entry whose mark SYS+0x5DC6+i is set is removed (0x40034A0C) and its file deleted (path 0x40034B78, f_unlink); then the marks are cleared, the index saved and reloaded, and SYS+0xF90 = 0.
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

void func_40034b78(u32 v);
void list_a_entry_remove(u32 v);
void list_a_marks_clear(void);
void list_a_index_save(void);
void list_a_index_load(void);

void list_a_delete_marked(void)
{
    int i;
    for (i = SS16(0xFA0) - 1; i >= 0; i--) {
        if (SYS8(0x5DC6 + i)) {
            u32 v = SYS16(0xFA6 + 2 * i);
            list_a_entry_remove(v);
            func_40034b78(v);
            f_unlink(PATHBUF);
        }
    }
    list_a_marks_clear();
    list_a_index_save();
    list_a_index_load();
    SYS8(0xF90) = 0;
}
