/*
 * settings_i_decode -- stock 0x400456AC (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, cursor = SYS+0x760, then settings_i_fields.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

void settings_i_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SYS32(0x760));
    settings_i_fields();
}
