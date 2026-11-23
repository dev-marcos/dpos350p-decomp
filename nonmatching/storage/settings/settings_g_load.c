/*
 * settings_g_load -- stock 0x40044760 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads record G from the flash address SYS+0x7E8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_g_load(void)
{
    persistence_read(0, SI(rec_g_flash_addr), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_g_fields();
}
