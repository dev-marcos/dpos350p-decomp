/*
 * settings_e_load -- stock 0x40043E34 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads record E. Without the magic 0x5A5555A5 (first start or erased flash) it rebuilds the defaults: 0x4003EFAC, then settings_slot_save for slots 1..5 (SYS_INFO+0x7C0), slot 1 saved in record F, SYS+0x7AC = 2 and record B saved.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "system.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_e_load(void)
{
    u32 i;
    persistence_read(0, SI(rec_e_flash_addr), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_e_fields();
    if (*(volatile u32 *)0x42E68AE0u == 0x5A5555A5u)
        return;
    factory_defaults();
    for (i = 1; i <= 5; i++) {
        SI(f_7c0) = i;
        settings_slot_save();
    }
    SI(f_7c0) = 1;
    settings_f_save();
    SI(menu_language) = 2;
    settings_b_save();
}
