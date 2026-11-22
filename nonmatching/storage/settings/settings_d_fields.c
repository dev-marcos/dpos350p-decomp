/*
 * settings_d_fields -- stock 0x40043CD0 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record D at 0x42E554B0: four words, then 300 entries of (word +0x10[i], double +0x4C0[i], double +0xE20[i]).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_d_fields(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E554B0u;
    volatile unsigned char *w = b + 0x10, *d = b + 0x4C0;
    int i;
    serializer_set_cursor(0);
    serializer_process_field(b, 4);
    serializer_process_field(b + 4, 4);
    serializer_process_field(b + 8, 4);
    serializer_process_field(b + 0xC, 4);
    for (i = 0; i < 300; i++) {
        serializer_process_field(w, 4);
        serializer_process_field(d, 8);
        serializer_process_field(d + 0x960, 8);
        d += 8;
        w += 4;
    }
}
