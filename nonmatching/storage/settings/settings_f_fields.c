/*
 * settings_f_fields -- stock 0x40043EC8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record F: the byte at 0x40545628 (SYS_INFO+0x7C0, the current slot).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "shared/storage.h"

void settings_f_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field((volatile void *)0x40545628u, 1);
}
