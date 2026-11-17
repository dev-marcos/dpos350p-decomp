/*
 * disk_format_and_reload -- stock 0x40051C04 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * disk_format, disk_mount, 0x4003486C, 0x400358B8, then 0x40036B00.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void disk_format(void);
void list_a_index_load(void);
void list_b_index_load(void);
void list_c_index_load(void);

void disk_format_and_reload(void)
{
    disk_format();
    disk_mount();
    list_a_index_load();
    list_b_index_load();
    list_c_index_load();
}
