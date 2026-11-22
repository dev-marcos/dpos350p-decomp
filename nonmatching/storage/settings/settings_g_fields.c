/*
 * settings_g_fields -- stock 0x400446E8 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record G: the three words at 0x42E3F984 + 0x2F64 / 0x2F68 / 0x2F6C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_g_fields(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E3F984u;
    serializer_set_cursor(0);
    serializer_process_field(b + 0x2F64, 4);
    serializer_process_field(b + 0x2F68, 4);
    serializer_process_field(b + 0x2F6C, 4);
}
