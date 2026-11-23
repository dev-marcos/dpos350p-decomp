/*
 * settings_block_448b4_decode -- stock 0x40045688 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, cursor = SYS+0x75C, then the field list 0x400448B4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

void settings_block_448b4_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SI(rec_h_start));
    settings_image_fields();
}
