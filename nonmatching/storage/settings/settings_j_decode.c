/*
 * settings_j_decode -- stock 0x400456D0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, cursor = SYS+0x764, then settings_j_fields.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

void settings_j_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SI(rec_j_start));
    settings_j_fields();
}
