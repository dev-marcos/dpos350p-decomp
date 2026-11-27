/*
 * list_c_index_create -- stock 0x40036958 (424 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Run when a list C file does not open: creates 0x4008FA2C, writes an empty index 0x4008E2EC (count 0, SYS+0x3DA52E = 1, zeroed entries) and a data file 0x4008E8D4 of SYS+0x758 * SYS+0x3DA544 halfwords. The data block is cleared with copy_u16 from address 0, as the firmware does.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void list_c_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)IMG(4008fa2c));
    f_chmod((const char *)IMG(4008fa2c), 2, 2);
    f_chmod((const char *)IMG(4008fa2c), 4, 4);
    f_open(fil, (const char *)IMG(4008e2ec), 0xB);
    f_chmod((const char *)IMG(4008e2ec), 2, 2);
    f_chmod((const char *)IMG(4008e2ec), 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SI(capture_slot_count));
    SI(list_current) = 1;
    SI(list_count) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
    f_lseek(fil, 4);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SI(capture_slot_count) * 2u, 0);
    f_close(fil);
    f_open(fil, (const char *)IMG(4008e8d4), 0xB);
    f_chmod((const char *)IMG(4008e8d4), 2, 2);
    f_chmod((const char *)IMG(4008e8d4), 4, 4);
    copy_u16((unsigned short *)(0x40544E68u + 0xD744u), 0, SI(capture_slot_count) * SI(capture_cells));
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xD744u), SI(capture_slot_count) * SI(capture_cells), 0);
    f_close(fil);
}
