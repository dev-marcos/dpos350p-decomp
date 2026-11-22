/*
 * settings_a_fields -- stock 0x400439A0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record A: the 64 bytes at 0x405454EC (SYS_INFO+0x684), one byte each.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_a_fields(void)
{
    volatile unsigned char *p = (volatile unsigned char *)0x405454ECu;
    int i;
    serializer_set_cursor(0);
    for (i = 0; i < 0x40; i++)
        serializer_process_field(p++, 1);
}
