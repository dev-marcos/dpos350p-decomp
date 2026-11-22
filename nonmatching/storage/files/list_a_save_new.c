/*
 * list_a_save_new -- stock 0x4003506C (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves a screenshot: screenshot_thumb_make, shows a message box (text pointer word at 0x42C3C994) at the screen centre plus its offsets, reloads the index, takes the next free number, builds its path and writes the file (bmp_save); when that returns non-zero the number is inserted and the index saved. Ends with rect_42e68c38_draw.
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

void screenshot_thumb_make(void);
void list_a_index_load(void);
void list_a_next_free(void);
void screenshot_path_make(u32 v);
int bmp_save(const char *path);
void list_fa4_insert_754(u32 v);
void list_a_index_save(void);
void rect_42e68c38_draw(void);

void list_a_save_new(void)
{
    const signed char *t = (const signed char *)(0x42C2FE74u + 0xCB14u);
    int x = SYS16(4) + (SYS16(8) >> 1);
    int y = SYS16(6) + (SYS16(0xA) >> 1);
    screenshot_thumb_make();
    message_box_draw(x + t[0], y + t[1], *(const u32 *)(t + 0xC), 0);
    list_a_index_load();
    list_a_next_free();
    screenshot_path_make(SYS16(0xF9C));
    if (bmp_save(PATHBUF)) {
        list_fa4_insert_754(SYS16(0xF9C));
        list_a_index_save();
    }
    rect_42e68c38_draw();
}
