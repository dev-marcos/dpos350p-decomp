/*
 * settings_image_encode -- stock 0x40045714 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Encode mode; block 0x400448B4 from cursor 0x32 (SYS+0x75C = 0x32), then records I and J, recording each section start (SYS+0x75C / 0x760 / 0x764), size (SYS+0x768 / 0x76C / 0x770) and the end (SYS+0x774); finally the header H at cursor 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);

void settings_image_fields(void);
void settings_i_fields(void);
void settings_j_fields(void);
void settings_h_fields(void);

void settings_image_encode(void)
{
    serializer_set_encode_mode();
    SYS32(0x75C) = 0x32;
    serializer_set_cursor(0x32);
    settings_image_fields();
    SYS32(0x768) = serializer_get_cursor() - SYS32(0x75C);
    SYS32(0x760) = serializer_get_cursor();
    settings_i_fields();
    SYS32(0x76C) = serializer_get_cursor() - SYS32(0x760);
    SYS32(0x764) = serializer_get_cursor();
    settings_j_fields();
    SYS32(0x770) = serializer_get_cursor() - SYS32(0x764);
    SYS32(0x774) = serializer_get_cursor();
    settings_h_fields();
}
