/*
 * settings_slot_save -- stock 0x4004435C (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves the current user slot (SYS_INFO+0x7C0 = 1..5): encode the field list 0x40043F54, then write 0x1000 bytes of the image to the flash address SYS+0x7C8 + 4 x (slot - 1). Stock quirk: for a slot outside 1..5 the address is whatever the caller left in r4; the C uses 0 and the tests keep the slot in range.
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

void settings_slot_save(void)
{
    serializer_set_encode_mode();
    slot_fields();
    persistence_write_read_modify_write(0, slot_addr(), SER_IMAGE, 0x1000);
}
