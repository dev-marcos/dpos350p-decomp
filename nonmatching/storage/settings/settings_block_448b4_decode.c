/*
 * settings_block_448b4_decode -- stock 0x40045688 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, cursor = SYS+0x75C, then the field list 0x400448B4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);

void settings_image_fields(void);

void settings_block_448b4_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SYS32(0x75C));
    settings_image_fields();
}
