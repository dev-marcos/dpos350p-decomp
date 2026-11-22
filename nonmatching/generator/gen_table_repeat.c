/*
 * gen_table_repeat -- stock 0x4004D09C (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Table descriptor at G+off (length, top, -, -, count at +6): one period = length / count samples; the period is inverted (top - sample) and repeated count times over the buffer G+0xE. Skipped while the touch service is busy.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "util.h"

void gen_table_repeat(u32 off, u32 r1, volatile touch_t *t)
{
    volatile unsigned short *e = (volatile unsigned short *)(GEN_G + off);
    int per = (int)e[0] / (int)e[3];
    u32 j;
    if (t->serviced != 0)
        return;
    if (per != 0) {
        unsigned short *p = GEN_BUF;
        int n = per;
        do {
            *p = e[1] - *p;
            p++;
        } while (--n);
    }
    if (e[3] > 1) {
        j = 1;
        do {
            unsigned short *dst = GEN_BUF + j * per;
            j++;
            copy_u16(dst, GEN_BUF, per);
        } while (j < e[3]);
    }
}
