/*
 * settings_g_save -- stock 0x4004472C (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves record G: encode, then write 0x1000 bytes to the flash address SYS+0x7E8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "shared/storage.h"

void settings_g_save(void)
{
    serializer_set_encode_mode();
    settings_g_fields();
    persistence_write_read_modify_write(0, SI(rec_g_flash_addr), SER_IMAGE, 0x1000);
}
