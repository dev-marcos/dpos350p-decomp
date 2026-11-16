/*
 * settings_h_decode -- stock 0x40045670 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, then settings_h_fields (the header at cursor 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);

void settings_h_fields(void);

void settings_h_decode(void)
{
    serializer_set_decode_mode();
    settings_h_fields();
}
