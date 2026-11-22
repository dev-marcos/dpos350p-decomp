/*
 * settings_e_fields -- stock 0x40043DBC (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record E: the four words at 0x42E68AE0 (the first is the magic 0x5A5555A5).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_e_fields(void)
{
    volatile unsigned char *p = (volatile unsigned char *)0x42E68AE0u;
    serializer_set_cursor(0);
    serializer_process_field(p, 4);
    serializer_process_field(p + 4, 4);
    serializer_process_field(p + 8, 4);
    serializer_process_field(p + 0xC, 4);
}
