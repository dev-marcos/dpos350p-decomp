/*
 * settings_j_fields -- stock 0x400455E4 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record J: when SYS+0x3E is set, SYS+0x2B0 halfwords from 0x42AC7CF4; when SYS+0x102 is set, SYS+0x2B0 halfwords from 0x42B29F44 (the count is re-read every step).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);

void settings_j_fields(void)
{
    u32 i;
    if (SYS8(0x3E))
        for (i = 0; i < SYS32(0x2B0); i++)
            serializer_process_field((volatile void *)(0x42AC7CF4u + 2 * i), 2);
    if (SYS8(0x102))
        for (i = 0; i < SYS32(0x2B0); i++)
            serializer_process_field((volatile void *)(0x42B29F44u + 2 * i), 2);
}
