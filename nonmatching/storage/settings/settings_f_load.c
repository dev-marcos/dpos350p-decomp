/*
 * settings_f_load -- stock 0x40043F20 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads settings record F: read 0x1000 bytes from the flash address SYS_INFO+0x7C4 into the image, then decode the field list.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "shared/storage.h"

void settings_f_load(void)
{
    persistence_read(0, SI(rec_f_flash_addr), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_f_fields();
}
