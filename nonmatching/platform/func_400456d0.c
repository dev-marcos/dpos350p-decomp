/*
 * settings_j_decode -- stock 0x400456D0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, cursor = SYS+0x764, then settings_j_fields.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);

void settings_j_fields(void);

void settings_j_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SYS32(0x764));
    settings_j_fields();
}
