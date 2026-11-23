/*
 * settings_i_fields -- stock 0x40045580 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record I: the halfwords SYS+0x109684 (w) and SYS+0x109686 (h), then w x h halfwords from SYS+0x109688 (count read after the two fields, signed; nothing when < 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

void settings_i_fields(void)
{
    int n, i;
    serializer_process_field(&SI(bmp_width), 2);
    serializer_process_field(&SI(bmp_height), 2);
    n = (int)((u32)SI(bmp_width) * (u32)SI(bmp_height));
    for (i = 0; i < n; i++)
        serializer_process_field(&SYS16(0x109688 + 2 * i), 2);
}
