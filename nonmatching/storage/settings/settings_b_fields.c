/*
 * settings_b_fields -- stock 0x40043A40 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record B: SYS_INFO+0x7AC (1 byte), +0x658 (4), +0 (4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_b_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field((volatile void *)(0x40544E68u + 0x7AC), 1);
    serializer_process_field((volatile void *)(0x40544E68u + 0x658), 4);
    serializer_process_field((volatile void *)0x40544E68u, 4);
}
