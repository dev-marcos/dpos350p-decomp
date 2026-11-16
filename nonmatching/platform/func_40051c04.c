/*
 * disk_format_and_reload -- stock 0x40051C04 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * disk_format, disk_mount, 0x4003486C, 0x400358B8, then 0x40036B00.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void disk_format(void);
void func_4003486c(void);
void func_400358b8(void);
void func_40036b00(void);

void disk_format_and_reload(void)
{
    disk_format();
    disk_mount();
    func_4003486c();
    func_400358b8();
    func_40036b00();
}
