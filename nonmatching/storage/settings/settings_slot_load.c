/*
 * settings_slot_load -- stock 0x400443D4 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads the current user slot: read 0x1000 bytes from SYS+0x7C8 + 4 x (slot - 1), then decode the field list 0x40043F54. Same out-of-range quirk as settings_slot_save (stock: the caller's r1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

static u32 slot_addr(void)
{
    u32 s = SI(f_7c0) - 1;
    return s <= 4 ? SYS32(0x7C8 + 4 * s) : 0;
}

void settings_slot_load(void)
{
    persistence_read(0, slot_addr(), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    slot_fields();
}
