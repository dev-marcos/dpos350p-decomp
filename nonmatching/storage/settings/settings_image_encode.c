/*
 * settings_image_encode -- stock 0x40045714 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Encode mode; block 0x400448B4 from cursor 0x32 (SYS+0x75C = 0x32), then records I and J, recording each section start (SYS+0x75C / 0x760 / 0x764), size (SYS+0x768 / 0x76C / 0x770) and the end (SYS+0x774); finally the header H at cursor 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

void settings_image_encode(void)
{
    serializer_set_encode_mode();
    SI(rec_h_start) = 0x32;
    serializer_set_cursor(0x32);
    settings_image_fields();
    SI(rec_h_size) = serializer_get_cursor() - SI(rec_h_start);
    SI(rec_i_start) = serializer_get_cursor();
    settings_i_fields();
    SI(rec_i_size) = serializer_get_cursor() - SI(rec_i_start);
    SI(rec_j_start) = serializer_get_cursor();
    settings_j_fields();
    SI(rec_j_size) = serializer_get_cursor() - SI(rec_j_start);
    SI(rec_hij_end) = serializer_get_cursor();
    settings_h_fields();
}
