/*
 * settings_b_load -- stock 0x40043AB4 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads record B; when the loaded SYS_INFO+0 differs from the value before, sets SYS+0x7AC = 2 and runs 0x400447C8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "system.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_b_load(void)
{
    u32 old = SYS32(0);
    persistence_read(0, SYS32(0x7E4), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_b_fields();
    if (SYS32(0) != old) {
        SYS8(0x7AC) = 2;
        factory_reset_keep_language();
    }
}
