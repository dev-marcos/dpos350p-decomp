/*
 * settings_h_fields -- stock 0x4004480C (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record H: the words SYS_INFO+0x75C, 0x768, 0x760, 0x76C, 0x764, 0x770, 0x774, in that order.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_h_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field(&SI(rec_h_start), 4);
    serializer_process_field(&SI(rec_h_size), 4);
    serializer_process_field(&SI(rec_i_start), 4);
    serializer_process_field(&SI(rec_i_size), 4);
    serializer_process_field(&SI(rec_j_start), 4);
    serializer_process_field(&SI(rec_j_size), 4);
    serializer_process_field(&SI(rec_hij_end), 4);
}
