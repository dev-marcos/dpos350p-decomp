/*
 * settings_i_fields -- stock 0x40045580 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record I: the halfwords SYS+0x109684 (w) and SYS+0x109686 (h), then w x h halfwords from SYS+0x109688 (count read after the two fields, signed; nothing when < 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);

void settings_i_fields(void)
{
    int n, i;
    serializer_process_field(&SYS16(0x109684), 2);
    serializer_process_field(&SYS16(0x109686), 2);
    n = (int)((u32)SYS16(0x109684) * (u32)SYS16(0x109686));
    for (i = 0; i < n; i++)
        serializer_process_field(&SYS16(0x109688 + 2 * i), 2);
}
